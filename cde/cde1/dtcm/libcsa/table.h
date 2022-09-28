/* $XConsortium: table.h /main/cde1_maint/3 1995/10/10 13:31:45 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _TABLE_H
#define _TABLE_H

#pragma ident "@(#)table.h	1.14 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "rtable4.h"
#include "entry.h"
#include "connection.h"

/*
 *  Interface to rpc calls of version 2-4
 */

extern CSA_return_code _DtCm_table_create(
				Calendar *cal,
				_DtCm_Connection *conn);

extern CSA_return_code _DtCm_table_get_access(
				Calendar *cal,
				_DtCm_Connection *conn,
				cms_access_entry **acclist);

extern CSA_return_code _DtCm_table_set_access(
				Calendar *cal,
				_DtCm_Connection *conn,
				CSA_access_list alist);

extern CSA_return_code _DtCm_table_lookup_reminder(
				Calendar *cal,
				_DtCm_Connection *conn,
				CSA_uint32 num_names,
				char **reminder_names,
				CSA_uint32 *num_rems,
				CSA_reminder_reference **rems);

extern CSA_return_code _DtCm_table_lookup_range(
				Calendar	*cal,
				_DtCm_Connection *conn,
				time_t		start1,
				time_t		start2,
				boolean_t	no_end_time_range,
				time_t		end1,
				time_t		end2,
				long		id,
				CSA_uint32	num_attrs,
				cms_attribute	*attrs,
				CSA_enum	*ops,
				_DtCm_libentry	**appts);

extern CSA_return_code _DtCm_table_lookup(
				char		 	*cname,
				_DtCm_Connection	*conn,
				Uid_4			*uids,
				Appt_4			**appt);

/*
 * This routine looks up events of a repeating sequence.
 * The caller either specifies entry and elist_r or
 * id and key. 
 * If entry is specified, events are converted to cms_entry structures
 * and returned in elist_r, otherwise; the caller is interested only
 * in the key of the first entry which will be returned in the cms_key
 * structure pointed to by key.
 */
extern CSA_return_code _DtCm_table_lookup_key_range(
				Calendar *cal,
				_DtCm_Connection *conn,
				_DtCm_libentry *entry,
				long id,
				time_t start,
				time_t end,
				cms_key *key,
				_DtCm_libentry **elist_r);

extern CSA_return_code _DtCm_table_insert(
				Calendar *cal,
				_DtCm_Connection *conn,
				CSA_uint32 num_attrs,
				CSA_attribute * attrs,
				_DtCm_libentry **entries);

extern CSA_return_code _DtCm_table_delete(
				Calendar *cal,
				_DtCm_Connection *conn,
				_DtCm_libentry *entry,
				CSA_enum scope);

extern CSA_return_code _DtCm_table_update(
				Calendar *cal,
				_DtCm_Connection *conn,
				_DtCm_libentry *oentry,
				CSA_uint32 num_attrs,
				CSA_attribute * attrs,
				CSA_enum scope,
				_DtCm_libentry **nentry);

extern CSA_return_code _DtCm_table_size(
				Calendar *cal,
				_DtCm_Connection *conn,
				int *size);

extern CSA_return_code _DtCm_table_unregister_target(
				_DtCm_Connection *conn,
				char *cal);

extern CSA_return_code _DtCm_table_register_target(
				_DtCm_Connection *conn,
				char *cal);

extern pid_t _DtCm_get_pid();
#endif
