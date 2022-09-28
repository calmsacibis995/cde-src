/* $XConsortium: api.c /main/cde1_maint/2 1995/09/06 08:20:36 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)api.c	1.41 96/11/12 Sun Microsystems, Inc."

/*
 * Implements the calendar manager API functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#ifdef SunOS
#include <stropts.h>
#endif
#include "csa.h"
#include "agent_p.h"
#include "entry.h"
#include "rpccalls.h"
#include "debug.h"
#include "attr.h"
#include "xtclient.h"
#include "misc.h"
#include "free.h"
#include "iso8601.h"
#include "match.h"

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
static CSA_return_code _handle_register_callback_ext(CSA_extension *ext);
static CSA_return_code _check_logon_ext(CSA_extension *ext);
static CSA_return_code _handle_logon_ext(Calendar *cal, CSA_extension *ext);
static CSA_return_code _check_list_calendars_ext(CSA_extension *ext);
static void _handle_list_calendars_ext(CSA_extension *ext, CSA_uint32 vers);
static CSA_return_code _handle_query_config_ext(CSA_extension *ext);
static void _handle_com_support_ext(CSA_extension *ext);
static void _set_calendar_in_entries(Calendar *cal, _DtCm_libentry *elist);

/******************************************************************************
 * Calendar Manager API
 ******************************************************************************/

/*
 * List calendars supported by a server
 */
extern CSA_return_code
csa_list_calendars(
	CSA_service_reference	calendar_service,
	CSA_uint32		*number_names,
	CSA_calendar_user	**calendar_names,
	CSA_extension		*list_calendars_exts)
{
	CSA_return_code	stat;
	CSA_uint32	vers;

	DP(("api.c: csa_list_calendars\n"));

	if (calendar_service == NULL || number_names == NULL ||
	    calendar_names == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* check extension */
	if (list_calendars_exts != NULL &&
	    ((stat = _check_list_calendars_ext(list_calendars_exts))
	     != CSA_SUCCESS)) {
		return (stat);
	}

	if ((stat = _DtCm_rpc_list_calendars(calendar_service, &vers,
	    number_names, calendar_names)) == CSA_SUCCESS) {

		if (list_calendars_exts)
			_handle_list_calendars_ext(list_calendars_exts, vers);
	}

	return (stat);
}

/*
 * Logon to a calendar.
 * Returns a calendar session handle.
 *
 * arguments not used by this implementation:
 * calendar_servce, password, character_set, caller_csa_version,
 * logon_extensions
 * 
 * arguments ignored for now
 * character_set, caller_csa_version
 *
 * user - only the calendar_address field is used, other fields are ignored
 *	- format is calendar_name@host
 */
extern CSA_return_code
csa_logon(
	CSA_service_reference	calendar_service,
	CSA_calendar_user	*user,
	CSA_string		password,
	CSA_string		character_set,
	CSA_string		caller_csa_version,
	CSA_session_handle	*session,
	CSA_extension		*logon_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;

	DP(("api.c: csa_logon\n"));

	/* check validity of arguments */
	if (user == NULL || user->calendar_address == NULL || session == NULL
	   || strchr(user->calendar_address, '@') == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if (logon_extensions != NULL &&
	    ((stat = _check_logon_ext(logon_extensions)) != CSA_SUCCESS)) {
		return (stat);
	}

	/* create calendar object */
	if ((cal = _DtCm_new_Calendar(user->calendar_address)) == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	/* open calendar */
	if ((stat = _DtCm_rpc_open_calendar(cal)) == CSA_SUCCESS &&
	    (logon_extensions == NULL ||
	    (stat = _handle_logon_ext(cal, logon_extensions)) == CSA_SUCCESS)) {

		_DtCm_put_calendar_in_list(cal);
		*session = (CSA_session_handle)cal;

	} else {
		_DtCm_free_Calendar(cal);
	}

	return (stat);
}

/*
 * Create a calendar.
 *
 * arguments not used by this implementation:
 * session - always ignored
 * add_calendar_extensions
 * 
 * user - only the calendar_address field is used, other fields are ignored
 *	- format is calendar_name@host
 */
extern CSA_return_code
csa_add_calendar(
	CSA_session_handle	session,
	CSA_calendar_user	*user,
	CSA_uint32		number_attributes,
	CSA_attribute		*calendar_attributes,
	CSA_extension		*add_calendar_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;
	char		*host;

	DP(("api.c: csa_add_calendar\n"));

	/* check validity of arguments */
	if (user == NULL || user->calendar_address == NULL ||
	    (host = strchr(user->calendar_address, '@')) == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* check add_calendar_extensions and return appropriate return code */
	/* no function extension is supported */
	if (add_calendar_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* create calendar object */
	if ((cal = _DtCm_new_Calendar(user->calendar_address)) == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	host++;
	if ((stat = _DtCm_rpc_get_server_version(host, &cal->rpc_version))
	    != CSA_SUCCESS) {
		_DtCm_free_Calendar(cal);
		return (stat);
	}

	/* check validity of attributes */
	if (number_attributes > 0 &&
	    (stat = _DtCm_check_cal_csa_attributes(cal->rpc_version - 1,
	    number_attributes, calendar_attributes, user->calendar_address,
	    _B_TRUE, _B_TRUE, _B_FALSE)) != CSA_SUCCESS) {
		_DtCm_free_Calendar(cal);
		return (stat);
	}

	/* create calendar */
	stat = _DtCm_rpc_create_calendar(cal, number_attributes,
		calendar_attributes);

	_DtCm_free_Calendar(cal);
	return (stat);
}

/*
 * Logoff a calendar.
 * The calendar handle becomes invalid
 *
 * argument not used by this implementation:
 * logoff_extensions
 */
extern CSA_return_code
csa_logoff(
	CSA_session_handle session,
	CSA_extension *logoff_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;

	DP(("api.c: csa_logoff\n"));

	/* check logoff_extensions and return appropriate return code */
	/* no function extension is supported */
	if (logoff_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/* unregister with server */
	if (cal->all_reasons)
		(void) _DtCm_rpc_unregister_client(cal, cal->all_reasons);

	_DtCm_remove_calendar_from_list(cal);

	return (CSA_SUCCESS);
}

/*
 * delete calendar
 */
extern CSA_return_code
csa_delete_calendar(
	CSA_session_handle	session,
	CSA_extension	*delete_calendar_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;

	DP(("api.c: csa_delete_calendar\n"));

	/* check extensions and return appropriate return code */
	/* no function extension is supported */
	if (delete_calendar_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	if (cal->rpc_version < _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION) {
		stat = CSA_E_NOT_SUPPORTED;
	} else
		stat = _DtCm_rpc_delete_calendar(cal);

	_DtCm_release_calendar(cal);
	return (stat);
}

extern CSA_return_code
csa_list_calendar_attributes(
	CSA_session_handle	session,
	CSA_uint32	*number_names,
	CSA_attribute_reference	**calendar_attributes_names,
	CSA_extension	*list_calendar_attributes_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;

	DP(("api.c: csa_list_calendar_attributes\n"));

	if (number_names == NULL || calendar_attributes_names == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* no function extension is supported */
	if (list_calendar_attributes_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	if (cal->rpc_version < _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION) {
		stat = _DtCm_list_old_cal_attr_names(cal, number_names,
			calendar_attributes_names);
	} else
		stat = _DtCm_rpc_list_calendar_attributes(cal, number_names,
			calendar_attributes_names);

	_DtCm_release_calendar(cal);
	return (stat);
}

extern CSA_return_code
csa_save(
	CSA_session_handle	session,
	CSA_string	archive_name,
	CSA_uint32	number_attributes,
	CSA_attribute	*attributes,
	CSA_enum	*operators,
	CSA_boolean	delete_entry,
	CSA_extension	*save_extensions)
{
	DP(("api.c: csa_save\n"));

	return (CSA_E_NOT_SUPPORTED);
}

extern CSA_return_code
csa_restore(
	CSA_session_handle	session,
	CSA_string	archive_name,
	CSA_uint32	number_attributes,
	CSA_attribute	*attributes,
	CSA_enum	*operators,
	CSA_extension	*restore_extensions)
{
	DP(("api.c: csa_restore\n"));

	return (CSA_E_NOT_SUPPORTED);
}

/*
 * If list_operators is NULL, the operator is default to be CSA_MATCH_EQUAL_TO
 * for all attributes.
 * *** might want to check operator for conflicts that won't result in no match
 */
extern CSA_return_code
csa_list_entries(
	CSA_session_handle	session,
	CSA_uint32	number_attributes,
	CSA_attribute	*entry_attributes,
	CSA_enum	*list_operators,
	CSA_uint32	*number_entries,
	CSA_entry_handle	**entries,
	CSA_extension	*list_entries_extensions)
{
	CSA_return_code		stat;
	Calendar		*cal;
	_DtCm_libentry		*elist;
	int			i;

	DP(("api.c: csa_list_entries\n"));

	if (entries == NULL || number_entries == NULL)
		return (CSA_E_INVALID_PARAMETER);
	else {
		*entries = NULL;
		*number_entries = 0;
	}

	/* check list_entries_extensions
	 * and return appropriate return code
	 * no function extension is supported
	 */
	if (list_entries_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	if (list_operators && (stat = _DtCm_check_operator(number_attributes,
	    entry_attributes, NULL, list_operators)) != CSA_SUCCESS)
		return (stat);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/* check data type */
	if ((stat = _DtCm_check_entry_attributes(cal->file_version,
	    number_attributes, entry_attributes, 0, _B_FALSE)) != CSA_SUCCESS) {
		/*
		 * if attribute not supported by old backends
		 * are specified, just fail the match,
		 * i.e. return NULL and CSA_SUCCESS
		 */
		if (stat == CSA_E_UNSUPPORTED_ATTRIBUTE)
			stat = CSA_SUCCESS;

		goto done;
	}

	/* lookup entries */
	if ((stat = _DtCm_rpc_lookup_entries(cal, number_attributes,
	    entry_attributes, list_operators, &elist)) == CSA_SUCCESS) {

		if (elist) {
			_set_calendar_in_entries(cal, elist);
			stat = _DtCm_libentry_to_entryh(elist, number_entries,
				entries);
		}
	}

done:
	_DtCm_release_calendar(cal);
	return (stat);
}

extern CSA_return_code
csa_list_entry_attributes(
	CSA_session_handle	session,
	CSA_entry_handle	entryh,
	CSA_uint32	*number_names,
	CSA_attribute_reference	**entry_attribute_names,
	CSA_extension	*list_entry_attributes_extensions)
{
	CSA_return_code	stat;
	_DtCm_libentry	*entry;
	Calendar	*cal;

	DP(("api.c: csa_list_entry_attributes\n"));

	if (number_names == NULL || entry_attribute_names == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* check list_entry_attributes_extensions
	 * and return appropriate return code
	 * no function extension is supported
	 */
	if (list_entry_attributes_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get appointment */
	if ((entry = _DtCm_get_libentry(entryh)) == NULL)
		return (CSA_E_INVALID_ENTRY_HANDLE);

	if ((stat = _DtCm_get_calendar((CSA_session_handle)entry->cal,
	    _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	stat = _DtCm_get_entry_attr_names(entry, number_names,
		entry_attribute_names);

	_DtCm_release_calendar(cal);
	return (stat);
}

extern CSA_return_code
csa_read_entry_attributes(
	CSA_session_handle	session,
	CSA_entry_handle	entryh,
	CSA_uint32	number_names,
	CSA_attribute_reference	*attribute_names,
	CSA_uint32	*number_attributes,
	CSA_attribute	**entry_attributes,
	CSA_extension	*read_entry_attributes_extensions)
{
	CSA_return_code	stat;
	_DtCm_libentry	*entry;
	Calendar	*cal;

	DP(("api.c: csa_read_entry_attributes\n"));

	if (number_attributes == 0 || entry_attributes == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* check read_entry_attributes_extensions
	 * and return appropriate return code
	 * no function extension is supported
	 */
	if (read_entry_attributes_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get entry object */
	if ((entry = _DtCm_get_libentry(entryh)) == NULL)
		return (CSA_E_INVALID_ENTRY_HANDLE);

	if ((stat = _DtCm_get_calendar((CSA_session_handle)entry->cal,
	    _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	if (number_names > 0) {
		stat = _DtCm_get_entry_attrs_by_name(entry, number_names,
			attribute_names, number_attributes, entry_attributes);
	} else {
		stat = _DtCm_get_all_entry_attrs(entry, number_attributes,
			entry_attributes);
	}

	_DtCm_release_calendar(cal);
	return (stat);
}

extern CSA_return_code
csa_free(CSA_buffer memory)
{
	DP(("api.c: csa_free\n"));

	return (_DtCm_free(memory));
}

extern CSA_return_code
csa_look_up(
	CSA_session_handle	session,
	CSA_calendar_user	*users,
	CSA_flags	look_up_flags,
	CSA_uint32	*number_users,
	CSA_calendar_user	**user_list,
	CSA_extension	*look_up_extensions)
{
	DP(("api.c: csa_look_up\n"));

	return (CSA_E_NOT_SUPPORTED);
}

extern CSA_return_code
csa_query_configuration(
	CSA_session_handle	session,
	CSA_enum	item,
	CSA_buffer	*reference,
	CSA_extension	*query_configuration_extensions)
{
	CSA_return_code	stat = CSA_SUCCESS;
	Calendar	*cal;
	DP(("api.c: csa_query_configuration\n"));

	if (item < CSA_CONFIG_CHARACTER_SET || item > CSA_CONFIG_VER_SPEC)
		return (CSA_E_INVALID_ENUM);

	if (reference == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* get calendar object */
	if (session) {
		if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal))
		    != CSA_SUCCESS)
		{
			return (stat);
		} else
			_DtCm_release_calendar(cal);
	}

	if (query_configuration_extensions) {
		if ((stat = _handle_query_config_ext(
		    query_configuration_extensions)) != CSA_SUCCESS)
			return (stat);
	}

	switch (item) {
	case CSA_CONFIG_DEFAULT_SERVICE:
	case CSA_CONFIG_DEFAULT_USER:
		*reference = NULL;
		break;

	case CSA_CONFIG_REQ_PASSWORD:
		*reference = (CSA_buffer)CSA_REQUIRED_NO;
		break;

	case CSA_CONFIG_REQ_SERVICE:
	case CSA_CONFIG_REQ_USER:
		*reference = (CSA_buffer)CSA_REQUIRED_YES;
		break;

	case CSA_CONFIG_UI_AVAIL:
		*reference = (CSA_buffer)CSA_FALSE;
		break;

	case CSA_CONFIG_VER_SPEC:
		*reference = (CSA_buffer)strdup(_DtCM_SPEC_VERSION_SUPPORTED);
		break;

	case CSA_CONFIG_CHARACTER_SET:
	case CSA_CONFIG_LINE_TERM:
	case CSA_CONFIG_VER_IMPLEM:
		stat = CSA_E_UNSUPPORTED_ENUM;
	}
	
	return (stat);
}

extern CSA_return_code
csa_read_calendar_attributes(
	CSA_session_handle	session,
	CSA_uint32	number_names,
	CSA_attribute_reference	*attribute_names,
	CSA_uint32	*number_attributes,
	CSA_attribute	**calendar_attributes,
	CSA_extension	*read_calendar_attributes_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;
	int		i;

	DP(("api.c: csa_read_calendar_attributes\n"));

	if (number_attributes == 0 || calendar_attributes == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* check read_calendar_attributes_extensions
	 * and return appropriate return code
	 * no function extension is supported
	 */
	if (read_calendar_attributes_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	_DtCm_reset_cal_attrs(cal);
	if (number_names > 0) {
		stat = _DtCm_get_cal_attrs_by_name(cal, number_names,
			attribute_names, number_attributes,
			calendar_attributes);
	} else {
		stat = _DtCm_get_all_cal_attrs(cal, number_attributes,
			calendar_attributes);
	}

	_DtCm_release_calendar(cal);
	return (stat);
}

extern CSA_return_code
csa_register_callback(
	CSA_session_handle	session,
	CSA_flags		reason,
	CSA_callback		callback,
	CSA_buffer		client_data,
	CSA_extension		*register_callback_extensions)
{
	CSA_return_code		stat;
	_DtCmCallbackEntry	*cb_entry;
	Calendar		*cal;
	boolean_t		async = _B_FALSE;

	/* must specify a callback function otherwise
	 * there is no point to make this call
	 */
	if (callback == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/* make sure some valid flags are set
	 * and only entry added, deleted or updated is specified for
	 * servers support up to version 4 rpc protocol
	 */
	if (reason == 0 || reason >= (CSA_CB_ENTRY_UPDATED << 1) ||
	    (cal->rpc_version < _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION &&
	     (reason & ~(CSA_CB_ENTRY_ADDED|CSA_CB_ENTRY_DELETED|
	     CSA_CB_ENTRY_UPDATED)))) {
		stat = CSA_E_INVALID_FLAG;
		goto done;
	}

	/* need to initialize agent before doing XtAppAddInput */
	_DtCm_init_agent();

	if (register_callback_extensions != NULL) {
		if ((stat = _handle_register_callback_ext(
		    register_callback_extensions)) == CSA_SUCCESS)
			async = _B_TRUE;
		else
			goto done;
	}

	/*
	 * register interest only if we have a new flag
	 * NOTE: It's OK to register more than once, even if it's
	 * the same calendar, same reason.  The customer is always right.
	 */
	if (((cal->all_reasons | reason) ^ cal->all_reasons) &&
	    (stat = _DtCm_rpc_register_client(cal, reason)) != CSA_SUCCESS)
		goto done;

	if ((cb_entry = (_DtCmCallbackEntry*)malloc(sizeof(_DtCmCallbackEntry)))
	    == NULL) {
		stat = CSA_E_INSUFFICIENT_MEMORY;
		goto done;
	}

	/* update info in calendar structure */
	cal->all_reasons |= reason;

	/* don't just do cal->async_process = async, since cal->async_process
	 * might have been set to _B_TRUE before.
	 */
	if (async == _B_TRUE)
		cal->async_process = _B_TRUE;
	if (cal->async_process == _B_TRUE)
		cal->do_reasons = cal->all_reasons;

	/* fill in the callback record */
	cb_entry->reason = reason;
	cb_entry->handler = callback;
	cb_entry->client_data = client_data;
	
	/* insert it at head of list */
	cb_entry->next = cal->cb_list;
	cb_entry->prev = (_DtCmCallbackEntry*) NULL;
	if (cal->cb_list != (_DtCmCallbackEntry*) NULL)
		cal->cb_list->prev = cb_entry;
	cal->cb_list = cb_entry;

done:
	_DtCm_release_calendar(cal);
	return (stat);
}

/*
 * csa_unregister_callback
 *
 * removes a previsouly registered callback from the callback list
 * of the specified calendar.
 */
extern CSA_return_code
csa_unregister_callback(
	CSA_session_handle	session,
	CSA_flags		reason,
	CSA_callback		handler,
	CSA_buffer		client_data,
	CSA_extension		*unregister_callback_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;
	boolean_t	match_one = _B_FALSE;
	boolean_t	match_all = _B_FALSE;
	CSA_flags	all = 0, unreg;
	_DtCmCallbackEntry *cb, *ncb;

	if (reason == 0 || reason >= (CSA_CB_ENTRY_UPDATED << 1))
		return (CSA_E_INVALID_FLAG);

	/* check unregister_callback_extensions
	 * and return appropriate return code
	 * no function extension is supported
	 */
	if (unregister_callback_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	if (handler == NULL && client_data == NULL)
		match_all = _B_TRUE;
	else if (handler == NULL)	/* but client data is not NULL */
		return (CSA_E_INVALID_PARAMETER);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/*
	 * Removal policy: If both handler and client data are NULL,
	 * match all callbacks for the specified reason.
	 * Otherwise, all of reason, handler and client data must
	 * match.
	 */
	cb = cal->cb_list;
	while (cb) {
		ncb = cb->next;

		if (!(reason & cb->reason) ||
		    (match_all == _B_FALSE && (handler != cb->handler ||
		    client_data != cb->client_data))) {
			all = all | cb->reason;
			cb = cb->next;
			continue;
		}

		match_one = _B_TRUE;

		if (cb->reason = (cb->reason | reason) ^ reason) {
			all = all | cb->reason;
		} else {
			/* remove entry */
			if (cb->prev != NULL)
				cb->prev->next = cb->next;
			if (cb->next != NULL)
				cb->next->prev = cb->prev;
			if (cb == cal->cb_list)
				cal->cb_list = cb->next;
			free(cb);
		}

		cb = ncb;
	}

	unreg = all ^ cal->all_reasons;
	cal->all_reasons = all;

	if (unreg)
		(void) _DtCm_rpc_unregister_client(cal, unreg);

	if (match_one == _B_TRUE)
		stat = CSA_SUCCESS;
	else
		stat = CSA_E_CALLBACK_NOT_REGISTERED;

	_DtCm_release_calendar(cal);
	return (stat);
}

CSA_return_code
csa_call_callbacks(
	CSA_session_handle	session,
	CSA_flags		reason,
	CSA_extension		*call_callbacks_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;

	DP(("api.c: csa_call_callbacks\n"));

	/* no function extension is supported */
	if (call_callbacks_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* to avoid deadlock, acquire get rpc lock first */
	_DtCm_lock_get_rpc_request();

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_TRUE, &cal)) != CSA_SUCCESS)
		goto done;

	/* set up information in calendar structure */
	if (cal->async_process == _B_FALSE)
		cal->do_reasons = reason;

	/* trigger callback mechanism */
	_DtCm_process_updates();

	if (cal->async_process == _B_FALSE)
		cal->do_reasons = 0;

	_DtCm_release_calendar(cal);
done:
	_DtCm_unlock_get_rpc_request();
	return (stat);
}

extern CSA_return_code
csa_update_calendar_attributes(
	CSA_session_handle	session,
	CSA_uint32	num_attrs,
	CSA_attribute	*attrs,
	CSA_extension	*update_calendar_attributes_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;
	int		i;

	DP(("api.c: csa_update_calendar_attributes\n"));

	if (num_attrs == 0 || attrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/*
	 * check update_calendar_attributes_extensions
	 * return appropriate return code
	 * no function extension is supported
	 */
	if (update_calendar_attributes_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/* check authority */
	if ((cal->file_version >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !(cal->access & (CSA_OWNER_RIGHTS | CSA_INSERT_CALENDAR_ATTRIBUTES |
	    CSA_CHANGE_CALENDAR_ATTRIBUTES))) ||
	    (cal->file_version < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !(cal->access & CSA_OWNER_RIGHTS))) {
		stat = CSA_E_NO_AUTHORITY;
		goto done;
	}

	/* check data type */
	if ((stat = _DtCm_check_cal_csa_attributes(cal->file_version,
	    num_attrs, attrs, cal->name, _B_TRUE, _B_FALSE, _B_TRUE))
	    == CSA_SUCCESS) {

		/* set attributes */
		stat = _DtCm_rpc_set_cal_attrs(cal, num_attrs, attrs);
	}

done:
	_DtCm_release_calendar(cal);
	return (stat);
}

extern CSA_return_code
csa_add_entry(
	CSA_session_handle	session,
	CSA_uint32	num_attrs,
	CSA_attribute	*attrs,
	CSA_entry_handle	*entry_r,
	CSA_extension	*add_entry_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;
	_DtCm_libentry	*entry;

	DP(("api.c: csa_add_entry\n"));

	if (num_attrs == 0 || attrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* no function extension is supported */
	if (add_entry_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/* check data type, readonly attributes, etc */
	if ((stat = _DtCm_check_entry_attributes(cal->file_version,
	    num_attrs, attrs, CSA_CB_ENTRY_ADDED, _B_TRUE)) != CSA_SUCCESS) {
		goto done;
	}

	/* insert in calendar */
	if ((stat = _DtCm_rpc_insert_entry(cal, num_attrs, attrs, &entry))
	    == CSA_SUCCESS) {

		if (entry_r != NULL) {
			if ((entry = _DtCm_convert_entry_wheader(entry))==NULL)
				stat = CSA_E_INSUFFICIENT_MEMORY;
			else {
				entry->cal = cal;
				*entry_r = (CSA_entry_handle)entry;
			}
		} else
			_DtCm_free_libentries(entry);
	}

done:
	_DtCm_release_calendar(cal);
	return (stat);
}

/*
 * The session argument is ignored in this implementation.
 */
extern CSA_return_code
csa_delete_entry(
	CSA_session_handle	session,
	CSA_entry_handle	entryh,
	CSA_enum	delete_scope,
	CSA_extension	*delete_entry_extensions)
{
	CSA_return_code	stat;
	_DtCm_libentry	*entry;
	Calendar	*cal;
	int		i;

	DP(("api.c: csa_delete_entry\n"));

	if (delete_scope < CSA_SCOPE_ALL || delete_scope > CSA_SCOPE_FORWARD)
		return (CSA_E_INVALID_ENUM);

	/* check delete_entry_extensions and return appropriate return code */
	/* no function extension is supported */
	if (delete_entry_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get entry object */
	if ((entry = _DtCm_get_libentry(entryh)) == NULL)
		return (CSA_E_INVALID_ENTRY_HANDLE);
	
	if ((stat = _DtCm_get_calendar((CSA_session_handle)entry->cal,
	    _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/* delete entry from calendar */
	stat = _DtCm_rpc_delete_entry(entry->cal, entry, delete_scope);

	_DtCm_release_calendar(cal);
	return (stat);
}

extern CSA_return_code
csa_free_time_search(
	CSA_session_handle	session,
	CSA_date_time_range	date_time_range,
	CSA_time_duration	time_duration,
	CSA_uint32		number_users,
	CSA_calendar_user	*users,
	CSA_free_time	**free_time,
	CSA_extension	*free_time_search_extensions)
{
	DP(("api.c: csa_free_time_search\n"));

	return (CSA_E_NOT_SUPPORTED);
}

extern CSA_return_code
csa_list_entry_sequence(
	CSA_session_handle	session,
	CSA_entry_handle	entryh,
	CSA_date_time_range	time_range,
	CSA_uint32		*number_entries,
	CSA_entry_handle	**entries,
	CSA_extension	*list_entry_sequence_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;
	_DtCm_libentry	*entry, *elist;
	cms_attribute	*rtype;
	cms_attribute	*rtimes;
	time_t		start = 0, end = 0;
	char *ptr;

	DP(("api.c: csa_list_entry_sequence\n"));

	if (number_entries == NULL || entries == NULL)
		return (CSA_E_INVALID_PARAMETER);
	else {
		*number_entries = 0;
		*entries = NULL;
	}

	/* no function extension is supported */
	if (list_entry_sequence_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get entry object */
	if ((entry = _DtCm_get_libentry(entryh)) == NULL)
		return (CSA_E_INVALID_ENTRY_HANDLE);
	
	if ((stat = _DtCm_get_calendar((CSA_session_handle)entry->cal,
	    _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/* check whether this entry repeats */
	/* fail it if it's not a repeating event */
	if (entry->cal->file_version < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		rtype = &entry->e->attrs[CSA_X_DT_ENTRY_ATTR_REPEAT_TYPE_I];
		if (rtype->value == NULL ||
	    	    rtype->value->item.sint32_value == CSA_X_DT_REPEAT_ONETIME)
		{
			stat = CSA_E_INVALID_PARAMETER;
			goto done;
		}
	} else {
		rtype = &entry->e->attrs[CSA_ENTRY_ATTR_RECURRENCE_RULE_I];
		if (rtype->value == NULL ||
		    rtype->value->item.string_value == NULL ||
		    *(rtype->value->item.string_value) == NULL) {
			stat = CSA_E_INVALID_PARAMETER;
			goto done;
		}
	}


	/* if this entry repeats indefinitely and time range is not
	 * specified, fail it
	 */
	if (time_range) {
		if (_csa_iso8601_to_range(time_range, &start, &end) != 0) {
			stat = CSA_E_INVALID_DATE_TIME;
			goto done;
		}
	}

	if (entry->cal->file_version < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION)
		rtimes = &entry->e->attrs[CSA_X_DT_ENTRY_ATTR_REPEAT_TIMES_I];
	else
		rtimes = &entry->e->attrs[CSA_ENTRY_ATTR_NUMBER_RECURRENCES_I];

	if (start == 0 && end == 0 &&
	    (rtimes->value && rtimes->value->item.uint32_value
	    == CSA_X_DT_DT_REPEAT_FOREVER)) {
		stat = CSA_E_INVALID_PARAMETER;
		goto done;
	}

	/* lookup sequence */
	if ((stat = _DtCm_rpc_enumerate_sequence(entry->cal, entry, start, end,
	    &elist)) == CSA_SUCCESS) {

		if (elist) {
			_set_calendar_in_entries(entry->cal, elist);
			stat = _DtCm_libentry_to_entryh(elist, number_entries,
					entries);
		}
	}

done:
	_DtCm_release_calendar(cal);
	return (stat);
}

/*
 * Due to the implementation of existing backends (versions 2-4)
 * which will unmanage any reminders that happens before the
 * the given tick, the user specified tick is ignore and
 * we will pass in the current time.
 */
extern CSA_return_code
csa_read_next_reminder(
	CSA_session_handle	session,
	CSA_uint32	number_names,
	CSA_attribute_reference	*reminder_names,
	CSA_date_time	given_time,
	CSA_uint32	*number_reminders,
	CSA_reminder_reference	**reminders,
	CSA_extension	*read_next_reminder_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;
	_DtCm_libentry	*eptr;
	int	i;
	time_t	timeval;

	DP(("api.c: csa_read_next_reminder\n"));

	if (number_reminders == 0 || reminders == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if (given_time == NULL || _csa_iso8601_to_tick(given_time, &timeval))
		return (CSA_E_INVALID_DATE_TIME);

	/*
	 * check read_next_reminder_extensions and
	 * return appropriate return code
	 * no function extension is supported
	 */
	if (read_next_reminder_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get calendar object */
	if ((stat = _DtCm_get_calendar(session, _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/* lookup reminders */
	if ((stat = _DtCm_rpc_lookup_reminder(cal, timeval,
	    number_names, reminder_names, number_reminders, reminders))
	    == CSA_SUCCESS) {

		/*
		 * link up all associated entries back to the calendar
		 */
		for (i = 0; i < *number_reminders; i++) {
			eptr = (_DtCm_libentry *)((*reminders)[i]).entry;

			/* link entry back to cal */
			eptr->cal = cal;
		}
	}

	_DtCm_release_calendar(cal);
	return (stat);
}

/*
 * These arguments are ignored in this implementation:
 * 	session, update_propagation
 */
extern CSA_return_code
csa_update_entry_attributes(
	CSA_session_handle	session,
	CSA_entry_handle	entry,
	CSA_enum		scope,
	CSA_boolean		update_propagation,
	CSA_uint32		num_attrs,
	CSA_attribute		*attrs,
	CSA_entry_handle	*new_entry,
	CSA_extension		*update_entry_attributes_extensions)
{
	CSA_return_code	stat;
	Calendar	*cal;
	_DtCm_libentry	*oentry, *tptr, *nentry;

	DP(("api.c: csa_update_entry_attributes\n"));

	if (scope < CSA_SCOPE_ALL || scope > CSA_SCOPE_FORWARD)
		return (CSA_E_INVALID_ENUM);

	if (num_attrs == 0 || attrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/*
	 * check update_entry_attributes_extensions and
	 * return appropriate return code
	 * no function extension is supported
	 */
	if (update_entry_attributes_extensions != NULL)
		return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

	/* get entry object */
	if ((oentry = _DtCm_get_libentry(entry)) == NULL)
		return (CSA_E_INVALID_ENTRY_HANDLE);
	
	if ((stat = _DtCm_get_calendar((CSA_session_handle)oentry->cal,
	    _B_FALSE, &cal)) != CSA_SUCCESS)
		return (stat);

	/* check data type, readonly attributes, etc */
	if ((stat = _DtCm_check_entry_attributes(oentry->cal->file_version,
	    num_attrs, attrs, CSA_CB_ENTRY_UPDATED, _B_TRUE)) != CSA_SUCCESS) {
		goto done;
	}

	/* change entry in calendar */
	if ((stat = _DtCm_rpc_update_entry(oentry->cal, oentry, num_attrs,
	    attrs, scope, &nentry)) == CSA_SUCCESS) {
		if (new_entry) {
			if ((nentry = _DtCm_convert_entry_wheader(nentry))
			    == NULL)
				stat = CSA_E_INSUFFICIENT_MEMORY;
			else {
				nentry->cal = oentry->cal;
				*new_entry = (CSA_entry_handle)nentry;
			}
		} else
			_DtCm_free_libentries(nentry);
	}

done:
	_DtCm_release_calendar(cal);
	return (stat);
}

/* ADD EVENT */
extern CSA_return_code
csa_add_event(
	CSA_service_reference	calendar_service,
	CSA_string		calendar_address,
	CSA_string		logon_user,
	CSA_string		logon_password,
	CSA_string		attendee,
	CSA_enum		attendee_priority,
	CSA_enum		attendee_status,
	CSA_boolean		attendee_rsvp_requested,
	CSA_date_time		start_date,
	CSA_date_time		end_date,
	CSA_string		organizer,
	CSA_string		sponsor,
	CSA_string		summary,
	CSA_string		description,
	CSA_string		recurrence_rule,
	CSA_string		exception_rule,
	CSA_string		subtype,
	CSA_enum		classification,
	CSA_string		delimiters,
	CSA_string		add_event_extensions)
{
	DP(("api.c: csa_add_event\n"));

	return (CSA_E_NOT_SUPPORTED);
}

/* ADD TODO */
CSA_return_code
csa_add_todo(
	CSA_service_reference	calendar_service,
	CSA_string		calendar_address,
	CSA_string		logon_user,
	CSA_string		logon_password,
	CSA_enum		attendee_priority,
	CSA_enum		attendee_status,
	CSA_boolean		attendee_rsvp_requested,
	CSA_date_time		start_date,
	CSA_date_time		due_date,
	CSA_uint32		priority,
	CSA_string		summary,
	CSA_string		description,
	CSA_enum		classification,
	CSA_string		delimiters,
	CSA_string		add_todo_extensions)
{
	DP(("api.c: csa_add_todo\n"));

	return (CSA_E_NOT_SUPPORTED);
}

/* ADD MEMO */
CSA_return_code
csa_add_memo(
	CSA_service_reference	calendar_service,
	CSA_string		calendar_address,
	CSA_string		logon_user,
	CSA_string		logon_password,
	CSA_date_time		start_date,
	CSA_string		summary,
	CSA_string		delimiters,
	CSA_string		add_memo_extensions)
{
	DP(("api.c: csa_add_memo\n"));

	return (CSA_E_NOT_SUPPORTED);
}

/******************************************************************************
 * static functions used within in the file
 ******************************************************************************/

static CSA_return_code
_handle_register_callback_ext(CSA_extension *ext)
{
	int		i;

	for (i = 0; ; i++) {
		if (ext[i].item_code == CSA_X_XT_APP_CONTEXT_EXT) {
			_DtCm_register_xtcallback(
				(XtAppContext)ext[i].item_data);
		} else
			return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

		if (ext[i].extension_flags & CSA_EXT_LAST_ELEMENT)
			break;
	}

	return (CSA_SUCCESS);
}

static CSA_return_code
_check_list_calendars_ext(CSA_extension *ext)
{
	int		i;

	for (i = 0; ; i++) {
		if (ext[i].item_code != CSA_X_DT_GET_SERVER_VERSION_EXT)
			return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

		if (ext[i].extension_flags & CSA_EXT_LAST_ELEMENT)
			break;
	}

	return (CSA_SUCCESS);
}

/*
 * validity should have been checked using _check_list_calendars_ext()
 */
static void
_handle_list_calendars_ext(CSA_extension *ext, CSA_uint32 vers)
{
	int		i;

	for (i = 0; ; i++) {
		/* we only support CSA_X_DT_GET_SERVER_VERSION_EXT */
		ext[i].item_data = vers;

		if (ext[i].extension_flags & CSA_EXT_LAST_ELEMENT)
			break;
	}
}

static CSA_return_code
_check_logon_ext(CSA_extension *ext)
{
	int		i;

	for (i = 0; ; i++) {
		if (ext[i].item_code != CSA_X_DT_GET_USER_ACCESS_EXT &&
		    ext[i].item_code != CSA_X_DT_GET_CAL_CHARSET_EXT &&
		    ext[i].item_code != CSA_X_DT_GET_SERVER_VERSION_EXT &&
		    ext[i].item_code != CSA_X_DT_GET_DATA_VERSION_EXT &&
		    ext[i].item_code != CSA_X_COM_SUPPORT_EXT)
			return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

		if (ext[i].extension_flags & CSA_EXT_LAST_ELEMENT)
			break;
	}

	return (CSA_SUCCESS);
}

/*
 * validity should have been checked using _check_logon_ext()
 */
static CSA_return_code
_handle_logon_ext(Calendar *cal, CSA_extension *ext)
{
	int		i;
	cms_attribute	*aptr;

	for (i = 0; ; i++) {
		switch (ext[i].item_code) {
		case CSA_X_DT_GET_USER_ACCESS_EXT:
			ext[i].item_data = cal->access;
			break;

		case CSA_X_DT_GET_DATA_VERSION_EXT:
			ext[i].item_data = cal->file_version;
			break;

		case CSA_X_DT_GET_SERVER_VERSION_EXT:
			ext[i].item_data = cal->rpc_version;
			break;
		
		case CSA_X_DT_GET_CAL_CHARSET_EXT:
			aptr = &cal->attrs[CSA_CAL_ATTR_CHARACTER_SET_I];
			if (aptr->value && aptr->value->item.string_value) {
				ext[i].item_data = strlen(aptr->value->\
							  item.string_value);

				if (ext[i].item_data > 0) {
					ext[i].item_reference =
						_DtCm_alloc_character_string(
							ext[i].item_data + 1);
					if (ext[i].item_reference)
						strcpy(ext[i].item_reference,
							aptr->value->item.\
							string_value);
					else
						return (CSA_E_INSUFFICIENT_MEMORY);

					ext[i].extension_flags |= CSA_EXT_OUTPUT;
				} else
					ext[i].item_reference = NULL;

			} else {
				ext[i].item_data = 0;
				ext[i].item_reference = NULL;
			}
			break;

		case CSA_X_COM_SUPPORT_EXT:
			_handle_com_support_ext(&ext[i]);
			break;
		}

		if (ext[i].extension_flags & CSA_EXT_LAST_ELEMENT)
			break;
	}

	return (CSA_SUCCESS);
}

static CSA_return_code
_handle_query_config_ext(CSA_extension *ext)
{
	int		i;
	int		com_support_index = -1;

	for (i = 0; ; i++) {
		if (ext[i].item_code == CSA_X_COM_SUPPORT_EXT) {
			com_support_index = i;
		} else
			return (CSA_E_UNSUPPORTED_FUNCTION_EXT);

		if (ext[i].extension_flags & CSA_EXT_LAST_ELEMENT)
			break;
	}

	if (com_support_index >= 0)
		_handle_com_support_ext(&ext[com_support_index]);

	return (CSA_SUCCESS);
}

static void
_handle_com_support_ext(CSA_extension *ext)
{
	int			i;
	CSA_X_COM_support	*xcom;

	for (i = 0, xcom = ext->item_reference; i < ext->item_data; i++) {
		switch (xcom[i].item_code) {
		case CSA_X_COM_SUPPORT_EXT:
		case CSA_X_XT_APP_CONTEXT_EXT:
		case CSA_XS_DT:
		case CSA_X_DT_GET_USER_ACCESS_EXT:
			xcom[i].flags = CSA_X_COM_SUPPORTED;
			break;

		case CSA_XS_COM:
		case CSA_X_UI_ID_EXT:
		default:
			xcom[i].flags = CSA_X_COM_NOT_SUPPORTED;
			break;
		}
	}
}

static void
_set_calendar_in_entries(Calendar *cal, _DtCm_libentry *elist)
{
	_DtCm_libentry *ptr;

	if (elist == NULL)
		return;
	else
		ptr = elist;

	while (ptr) {
		ptr->cal = cal;
		ptr = ptr->next;
	}
}

