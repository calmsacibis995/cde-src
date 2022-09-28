/* $XConsortium: rpccalls.h /main/cde1_maint/2 1995/10/10 13:30:41 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _RPCCALLS_H
#define _RPCCALLS_H

#pragma ident "@(#)rpccalls.h	1.12 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "csa.h"
#include "entry.h"

/*
 * interface to cm rpc calls
 */

extern CSA_return_code _DtCm_rpc_open_calendar(Calendar *cal);

extern CSA_return_code _DtCm_rpc_create_calendar(Calendar *cal,
						 CSA_uint32 num_attrs,
						 CSA_attribute * attrs);

extern CSA_return_code _DtCm_rpc_delete_calendar(Calendar *cal);

extern CSA_return_code _DtCm_rpc_register_client(Calendar *cal,
						 CSA_flags type);

extern CSA_return_code _DtCm_rpc_unregister_client(Calendar *cal,
						   CSA_flags type);

extern CSA_return_code _DtCm_rpc_enumerate_sequence(Calendar *cal,
						    _DtCm_libentry *entry,
						    time_t start,
						    time_t end,
						    _DtCm_libentry **elist_r);

extern CSA_return_code _DtCm_rpc_lookup_entries(Calendar *cal,
						CSA_uint32 size,
						CSA_attribute * attrs,
						CSA_enum *ops,
						_DtCm_libentry **entries_r);

extern CSA_return_code _DtCm_rpc_lookup_entry_by_id(Calendar *cal,
						    _DtCm_libentry *entry);

extern CSA_return_code _DtCm_rpc_lookup_reminder(Calendar *cal,
						 time_t tick,
						 CSA_uint32 num_names,
						 char **reminder_names,
						 CSA_uint32 *num_rems,
						 CSA_reminder_reference **rems);

extern CSA_return_code _DtCm_rpc_set_cal_attrs(Calendar *cal,
					       CSA_uint32 num_attrs,
					       CSA_attribute * attrs);

extern CSA_return_code _DtCm_rpc_get_cal_attrs(Calendar	*cal,
					       int		index,
					       CSA_uint32	num_attrs,
					       char		**names);

extern CSA_return_code _DtCm_rpc_insert_entry(Calendar *cal,
					      CSA_uint32 num_attrs,
					      CSA_attribute * attrs,
					      _DtCm_libentry **entry_r);

extern CSA_return_code _DtCm_rpc_delete_entry(Calendar *cal,
					      _DtCm_libentry *entry,
					      CSA_enum scope);

extern CSA_return_code _DtCm_rpc_update_entry(Calendar *cal,
					      _DtCm_libentry *oentry,
					      CSA_uint32 num_attrs,
					      CSA_attribute * attrs,
					      CSA_enum scope,
					      _DtCm_libentry **nentry);

extern CSA_return_code _DtCm_rpc_list_calendar_attributes(Calendar	*cal,
							  CSA_uint32	*number_names,
							  char		***names_r);

extern CSA_return_code _DtCm_rpc_list_calendars(char		*location,
						CSA_uint32	*svc_version,
						CSA_uint32	*number_names,
						CSA_calendar_user **names_r);

extern CSA_return_code _DtCm_do_unregistration(_DtCm_Connection *conn,
					       char *cal,
					       CSA_flags update_type);

extern CSA_return_code _DtCm_do_registration(_DtCm_Connection *conn,
					     char *cal,
					     CSA_flags update_type);

extern CSA_return_code _DtCm_rpc_get_server_version(char	*host,
						    int	*version);

#endif

