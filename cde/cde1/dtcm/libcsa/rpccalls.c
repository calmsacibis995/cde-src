/* $XConsortium: rpccalls.c /main/cde1_maint/3 1995/10/10 13:30:29 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)rpccalls.c	1.49 97/01/31 Sun Microsystems, Inc."

/*****************************************************************************
 * interface to cm rpc calls
 * - this files make calls to v5 backend
 * - calls to v4 and before are implemented in table.c
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>  		/* define MAXHOSTNAMELEN */
#include <sys/utsname.h>	/* SYS_NMLN */
#include <unistd.h>
#ifdef SunOS
#include <sys/systeminfo.h>
#endif
#include "debug.h"
#include "rtable4.h" 
#include "cm.h"
#include "attr.h"
#include "connection.h"
#include "convert5-4.h"
#include "convert4-5.h"
#include "rpccalls.h"
#include "table.h"
#include "agent.h"
#include "cmcb.h"
#include "cmsdata.h"
#include "iso8601.h"
#include "misc.h"
#include "updateattrs.h"
#include "free.h"
#include "lutil.h"
#include "laccess.h"
#include "match.h"
#include "appt4.h"

extern u_long	_DtCm_transient2;

static char *nullstr = "";

/*****************************************************************************
 * forward declaration of static functions
 *****************************************************************************/
static CSA_return_code csa2cmsattrs(CSA_uint32 num_attrs,
					CSA_attribute *csaattrs,
					CSA_enum *ops, CSA_uint32 *newnum,
					cms_attribute **newattrs,
					CSA_enum **newops);
static void free_cmsattrs(CSA_uint32 num_attrs, cms_attribute *attrs);
static CSA_return_code csa2cmsnames(CSA_uint32 num_names, char **reminder_names,
					CSA_uint32 *newnum,
					cms_attr_name **newnames);
static CSA_return_code _GetV4UserAccess(Calendar *cal, _DtCm_Connection *conn,
					cms_access_entry *alist);
static CSA_return_code _get_entry_details(Calendar *cal, _DtCm_Connection conn,
					cms_reminder_ref *cmsrems,
					CSA_uint32 *num_rems,
					CSA_reminder_reference **csarems);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

extern CSA_return_code
_DtCm_rpc_open_calendar(Calendar *cal)
{
	CSA_return_code		stat;
	_DtCm_Client_Info	*ci;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_open_calendar\n"));

	if (cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}

	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_open_args	arg;
		cms_open_res	res;
		enum clnt_stat	rstat;

		arg.cal = cal->name;
		arg.pid = _DtCm_get_pid();
		memset((char *)&res, 0, sizeof(res));

		conn.retry = 1;
		rstat = cms_open_calendar_5(&arg, &res, &conn);

		if (rstat == RPC_SUCCESS) {
			if ((stat = res.stat) == CSA_SUCCESS) {
				if (res.user_access == 0) {
					stat = CSA_E_NO_AUTHORITY;
				} else {
					cal->rpc_version = res.svr_vers;
					cal->file_version = res.file_vers;
					cal->access = res.user_access;

					stat = _DtCmUpdateAttributes(
						res.num_attrs, res.attrs,
						&cal->num_attrs, &cal->attrs,
						&cal->cal_tbl, _B_TRUE, _B_FALSE);
				}
			}

			xdr_free((xdrproc_t)xdr_cms_open_res, (char *)&res);
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}
	} else {
		_DtCm_libentry		*entries;
		cms_access_entry	*alist;

		cal->rpc_version = ci->vers_out;
		cal->file_version = ci->vers_out - 1;
 
		/*
		 * call lookup_range to check the existence of
		 * the calendar
		 */
		if ((stat = _DtCm_table_lookup_range(cal, &conn, 0, 0, _B_TRUE,
		    0, 0, 0, 0, NULL, NULL, &entries)) == CSA_SUCCESS) {
			if (entries) _DtCm_free_libentries(entries);

			if ((stat = _DtCm_table_get_access(cal, &conn, &alist))
			    == CSA_SUCCESS) {
				stat = _GetV4UserAccess(cal, &conn, alist);
				_DtCm_free_cms_access_entry(alist);

				if (cal->access == 0)
					stat = CSA_E_NO_AUTHORITY;
			}
		}
	}

	pthread_mutex_unlock(&ci->lock_handle);

	return (stat);
}

extern CSA_return_code
_DtCm_rpc_create_calendar(
	Calendar *cal,
	CSA_uint32 num_attrs,
	CSA_attribute * attrs)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_create_calendar\n"));

	if (cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_create_args args;
		enum clnt_stat	rstat;
		CSA_uint32	num_cmsattrs;
		cms_attribute	*cmsattrs;

		if ((stat = csa2cmsattrs(num_attrs, attrs, NULL, &num_cmsattrs,
		    &cmsattrs, NULL)) != CSA_SUCCESS)
			goto done;

		args.cal	= cal->name;
		args.char_set	= nullstr;
		args.pid	= _DtCm_get_pid();
		args.num_attrs	= num_cmsattrs;
		args.attrs	= cmsattrs;

		conn.retry = 0;
		rstat = cms_create_calendar_5(&args, &stat, &conn);

		if (num_cmsattrs) free_cmsattrs(num_cmsattrs, cmsattrs);

		if (rstat != RPC_SUCCESS)
			stat = _DtCm_clntstat_to_csastat(rstat);

	} else if (ci->vers_out == TABLEVERS_4) {

		if ((stat = _DtCm_table_create(cal, &conn)) == CSA_SUCCESS &&
		    num_attrs > 0) {

			int	i;

			/*
			 * for old backends, the only settable
			 * calendar attribute is access list
			 */
			for (i = num_attrs - 1; i >= 0; i--) {
				if (attrs[i].name)
					break;
			}
			if (attrs[i].value == NULL)
				stat = _DtCm_table_set_access(cal, &conn, NULL);
			else
				stat = _DtCm_table_set_access(cal, &conn,
					attrs[i].value->item.access_list_value);
		}

	} else if (ci->vers_out < TABLEVERS_4) {
		_DtCm_libentry		*entries;
		char			*ptr, *owner;
		boolean_t		isuser;

		/* check to make sure user is not creating a calendar
		 * using another user's name
		 */
		if (ptr = strchr(cal->name, '@')) *ptr = NULL;
		if (_DtCmIsUserName(cal->name) == _B_TRUE) {

			if ((owner = _DtCmGetUserName()) == NULL)
				stat = CSA_E_FAILURE;

			if (strcmp(cal->name, owner))
				stat = CSA_E_NO_AUTHORITY;
		} else
			stat = CSA_E_INVALID_PARAMETER;
		if (ptr) *ptr = '@';

		/*
		 * for v2 and v3 servers, calling a lookup routine has
		 * the side effect of having a calendar created
		 * the calendar
		 */
		if (stat == CSA_SUCCESS &&
		    (stat = _DtCm_table_lookup_range(cal, &conn, 0, 0, _B_TRUE,
		    0, 0, 0, 0, NULL, NULL, &entries)) == CSA_SUCCESS) {
			if (entries) _DtCm_free_libentries(entries);
		}
	}

done:
	pthread_mutex_unlock(&ci->lock_handle);

	return (stat);
}

extern CSA_return_code
_DtCm_rpc_delete_calendar(Calendar *cal)
{
	CSA_return_code		stat;
	_DtCm_Client_Info	*ci;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_delete_calendar\n"));

	if (cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_remove_args	arg;
		enum clnt_stat	rstat;

		arg.cal = cal->name;
		arg.pid = _DtCm_get_pid();

		conn.retry = 0;
		rstat = cms_remove_calendar_5(&arg, &stat, &conn);

		if (rstat != RPC_SUCCESS)
			stat = _DtCm_clntstat_to_csastat(rstat);

	} else
		stat = CSA_E_NOT_SUPPORTED;

	pthread_mutex_unlock(&ci->lock_handle);

	return (stat);
}

extern CSA_return_code
_DtCm_rpc_register_client(Calendar *cal, CSA_flags update_type)
{
	CSA_return_code		stat;
	_DtCm_Client_Info	*ci;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_register_client\n"));

	if (cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;
	conn.retry = 1;
	stat = _DtCm_do_registration(&conn, cal->name, update_type);

	pthread_mutex_unlock(&ci->lock_handle);

	return (stat);
}

extern CSA_return_code
_DtCm_rpc_unregister_client(Calendar *cal, CSA_flags update_type)
{
	CSA_return_code		stat;
	_DtCm_Client_Info	*ci;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_unregister_client\n"));

	if (cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;
	conn.retry = 1;
	stat = _DtCm_do_unregistration(&conn, cal->name, update_type);

	pthread_mutex_unlock(&ci->lock_handle);

	return (stat);
}

extern CSA_return_code
_DtCm_rpc_lookup_reminder(
	Calendar *cal,
	time_t tick,
	CSA_uint32 num_names,
	char **reminder_names,
	CSA_uint32 *num_rems,
	CSA_reminder_reference **rems)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_lookup_reminder\n"));

	if (cal == NULL || num_rems == 0 || rems == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_reminder_args	args;
		cms_reminder_res	res;
		enum clnt_stat		rstat;
		CSA_uint32	num_cmsnames;
		cms_attr_name	*cmsnames;

		if ((stat = csa2cmsnames(num_names, reminder_names,
		    &num_cmsnames, &cmsnames)) != CSA_SUCCESS)
			goto done;

		args.cal	= cal->name;
		args.tick	= tick;
		args.num_names	= num_cmsnames;
		args.names	= cmsnames;
		memset((char *)&res, 0, sizeof(res));

		conn.retry = 1;
		rstat = cms_lookup_reminder_5(&args, &res, &conn);

		if (cmsnames)
			free(cmsnames);

		if (rstat == RPC_SUCCESS) {
			if ((stat = res.stat) == CSA_SUCCESS) {
				if (res.rems == NULL) {
					*num_rems = 0;
					*rems = NULL;
				} else
					stat = _get_entry_details(cal, conn,
						res.rems, num_rems, rems);
			}
			xdr_free((xdrproc_t)xdr_cms_reminder_res, (char *)&res);
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}
	} else {
 
		stat = _DtCm_table_lookup_reminder(cal, &conn, num_names,
			reminder_names, num_rems, rems);
	}

done:
	pthread_mutex_unlock(&ci->lock_handle);

        return(stat);
}

/*
 * Lookup calendar entries.
 * If it's an old server, will try to match the attributes in the library;
 * but if attrlist contains attributes not supported by the old backend,
 * will just return CSA_E_UNSUPPORTED_ATTRIBUTE.
 */
extern CSA_return_code
_DtCm_rpc_lookup_entries(
	Calendar *cal,
	CSA_uint32 num_attrs,
	CSA_attribute * attrs,
	CSA_enum *ops,
	_DtCm_libentry **elist_r)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_lookup_entries\n"));

	if (cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;
	*elist_r = NULL;

	if (ci->vers_out == TABLEVERS) {
		cms_lookup_entries_args	args;
		cms_entries_res		res;
		enum clnt_stat		rstat;
		CSA_uint32		num_cmsattrs;
		cms_attribute		*cmsattrs;
		CSA_enum		*newops;

		if ((stat = csa2cmsattrs(num_attrs, attrs, ops, &num_cmsattrs,
		    &cmsattrs, &newops)) != CSA_SUCCESS)
			goto done;

		args.cal = cal->name;
		args.char_set = nullstr;
		args.num_attrs = num_cmsattrs;
		args.attrs = cmsattrs;
		args.ops = newops;
		memset((char *)&res, 0, sizeof(res));

		conn.retry = 1;
		rstat = cms_lookup_entries_5(&args, &res, &conn);

		free_cmsattrs(num_cmsattrs, cmsattrs);
		if (newops) free(newops);

		if (rstat == RPC_SUCCESS) {
			if ((stat = res.stat) == CSA_SUCCESS) {
				stat = _DtCmCmsentriesToLibentries(
					&cal->entry_tbl, res.entries, elist_r);
			}

			xdr_free((xdrproc_t)xdr_cms_entries_res, (char *)&res);
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}
	} else {
		time_t		start1, start2, end1, end2;
		long		id;
		boolean_t	no_start_time_range;
		boolean_t	no_end_time_range;
		boolean_t	no_match;
		CSA_uint32	hnum;
		cms_attribute	*hattrs;
		CSA_enum	*hops;
		cms_key		key;
		_DtCm_libentry	*entry;
		Uid_4		key4;
		Appt_4		*appt;
 
		/* hash the matching criteria */
		if ((stat = _DtCmHashCriteria(cal->entry_tbl, num_attrs, attrs,
		    NULL, ops, &no_match, &no_start_time_range,
		    &no_end_time_range, &start1, &start2, &end1, &end2, &id,
		    &hnum, &hattrs, &hops)) == CSA_E_INVALID_ATTRIBUTE)
		{
			stat = CSA_SUCCESS;
			goto done;
		} else if (stat != CSA_SUCCESS || no_match == _B_TRUE) {
			goto done;
		}

		if (id > 0 && no_start_time_range && no_end_time_range) {
			/* get the key for the first event */
			key.id = 0;
			if ((stat = _DtCm_table_lookup_key_range(cal, &conn,
			    NULL, id, _DtCM_BOT, _DtCM_EOT, &key, NULL))
			    == CSA_SUCCESS && key.id > 0) {

			    /* get the detail */
			    if ((stat = _DtCm_make_libentry(NULL, &entry))
				== CSA_SUCCESS) {
				    entry->e->key = key;

				    key4.appt_id.tick = key.time;
				    key4.appt_id.key = key.id;
				    key4.next = NULL;

				    if ((stat = _DtCm_table_lookup(cal->name,
					&conn, &key4, &appt)) == CSA_SUCCESS) {
					    stat = _DtCm_appt4_to_attrs(
						cal->name, appt,
						entry->e->num_attrs,
						entry->e->attrs, _B_FALSE);
					    _DtCm_free_appt4(appt);
				    }

				    if (stat == CSA_SUCCESS)
					    *elist_r = entry;
				    else
					    _DtCm_free_libentries(entry);
			    }
			}
		} else {
			stat = _DtCm_table_lookup_range(cal, &conn, start1,
				start2, no_end_time_range, end1, end2,
				id, hnum, hattrs, hops, elist_r);
		}

		_DtCmFreeHashedArrays(hnum, hattrs, hops);
	}

done:
	pthread_mutex_unlock(&ci->lock_handle);

        return(stat);
}

extern CSA_return_code
_DtCm_rpc_enumerate_sequence(
	Calendar *cal,
	_DtCm_libentry *entry,
	time_t start,
	time_t end,
	_DtCm_libentry **elist_r)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;
	int			i;

	DP(("rpccalls.c: _DtCm_rpc_enumerate_sequence\n"));

	if (cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;
	*elist_r = NULL;

	if (ci->vers_out == TABLEVERS) {
		cms_enumerate_args	args;
		cms_entries_res		res;
		enum clnt_stat		rstat;

		args.cal = cal->name;
		args.id = entry->e->key.id;
		args.start = start;
		args.end = end;
		memset((char *)&res, 0, sizeof(res));

		conn.retry = 1;
		rstat = cms_enumerate_sequence_5(&args, &res, &conn);

		if (rstat == RPC_SUCCESS) {
			if ((stat = res.stat) == CSA_SUCCESS) {
				stat = _DtCmCmsentriesToLibentries(
					&cal->entry_tbl, res.entries, elist_r);
			}

			xdr_free((xdrproc_t)xdr_cms_entries_res, (char *)&res);
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}

	} else {
 
		stat = _DtCm_table_lookup_key_range(cal, &conn, entry, 0,
			start, end, NULL, elist_r);
	}

	pthread_mutex_unlock(&ci->lock_handle);

        return(stat);
}

extern CSA_return_code
_DtCm_rpc_get_cal_attrs(
	Calendar	*cal,
	int		index,
	CSA_uint32	num_names,
	char		**names)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;
	cms_attribute_value	val;

	DP(("rpccalls.c: _DtCm_rpc_get_cal_attrs\n"));

	if (cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

        if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_get_cal_attr_args	arg;
		cms_get_cal_attr_res	res;
		enum clnt_stat		rstat;
		CSA_uint32		num_cmsnames;
		cms_attr_name		*cmsnames;

		if ((stat = csa2cmsnames(num_names, names,
		    &num_cmsnames, &cmsnames)) != CSA_SUCCESS)
			goto done;

		arg.cal = cal->name;
		arg.num_names = num_cmsnames;
		arg.names = cmsnames;
		memset((char *)&res, 0, sizeof(res));

		conn.retry = 1;
		rstat = cms_get_calendar_attr_5(&arg, &res, &conn);

		if (rstat == RPC_SUCCESS) {
			if ((stat = res.stat) == CSA_SUCCESS) {
				if ((stat = _DtCmUpdateAttributes(
				    res.num_attrs, res.attrs, &cal->num_attrs,
				    &cal->attrs, &cal->cal_tbl, _B_TRUE,
				    _B_TRUE)) == CSA_SUCCESS)
					cal->got_attrs = _B_TRUE;
			}

			xdr_free((xdrproc_t)xdr_cms_get_cal_attr_res, (char *)&res);
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}
	} else {

		switch (index) {
		case CSA_CAL_ATTR_ACCESS_LIST_I:
			if ((stat = _DtCm_table_get_access(cal, &conn,
			    &val.item.access_list_value)) == CSA_SUCCESS) {
				val.type = CSA_VALUE_ACCESS_LIST;
				stat = _DtCmUpdateAccessListAttrVal(&val,
					&cal->attrs\
					[CSA_CAL_ATTR_ACCESS_LIST_I].value);
				_DtCm_free_cms_access_entry(val.item.\
					access_list_value);
			}
			break;

		case CSA_CAL_ATTR_NUMBER_ENTRIES_I:
			if ((stat = _DtCm_table_size(cal, &conn,
			    (int*)&val.item.uint32_value)) == CSA_SUCCESS) {
				val.type = CSA_VALUE_UINT32;
				stat = _DtCmUpdateSint32AttrVal(&val,
					&cal->attrs\
					[CSA_CAL_ATTR_NUMBER_ENTRIES_I].value);
			}
			break;

		}
	}

done:
	pthread_mutex_unlock(&ci->lock_handle);

        return(stat);
}

extern CSA_return_code
_DtCm_rpc_set_cal_attrs(
	Calendar	*cal,
	CSA_uint32	num_attrs,
	CSA_attribute	*attrs)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;
	int			i;

	DP(("rpccalls.c: _DtCm_rpc_set_cal_attrs\n"));

	if (cal == NULL || attrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

        if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_set_cal_attr_args	args;
		enum clnt_stat		rstat;
		CSA_uint32	num_cmsattrs;
		cms_attribute	*cmsattrs;

		if ((stat = csa2cmsattrs(num_attrs, attrs, NULL, &num_cmsattrs,
		    &cmsattrs, NULL)) != CSA_SUCCESS)
			goto done;

		if (num_cmsattrs == 0) {
			stat = CSA_E_INVALID_PARAMETER;
			goto done;
		}

		args.cal = cal->name;
		args.pid = _DtCm_get_pid();
		args.num_attrs = num_cmsattrs;
		args.attrs = cmsattrs;

		conn.retry = 0;
		rstat = cms_set_calendar_attr_5(&args, &stat, &conn);

		free_cmsattrs(num_cmsattrs, cmsattrs);

		if (rstat == RPC_SUCCESS) {
			if (stat == CSA_SUCCESS) {
				/* purge old values */
				_DtCm_free_cms_attribute_values(cal->num_attrs,
					cal->attrs);
			}
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}

	} else {
		int	i;

		/*
		 * CSA_CAL_ATTR_ACCESS_LIST is the only settable attribute
		 */
		for (i = num_attrs - 1; i >= 0; i--) {
			if (attrs[i].name)
				break;
		}
		if (attrs[i].value == NULL)
			stat = _DtCm_table_set_access(cal, &conn, NULL);
		else
			stat = _DtCm_table_set_access(cal, &conn,
				attrs[i].value->item.access_list_value);
	}

done:
	pthread_mutex_unlock(&ci->lock_handle);

	return (stat);
}

extern CSA_return_code
_DtCm_rpc_insert_entry(
	Calendar *cal,
	CSA_uint32 num_attrs,
	CSA_attribute * attrs,
	_DtCm_libentry **entry_r)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_insert_entry;\n"));

	if (cal == NULL || num_attrs == 0 || attrs == NULL || entry_r == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_entry_res 	res;
		cms_insert_args	args;
		enum clnt_stat	rstat;
		CSA_uint32	num_cmsattrs;
		cms_attribute	*cmsattrs;

		if ((stat = csa2cmsattrs(num_attrs, attrs, NULL, &num_cmsattrs,
		    &cmsattrs, NULL)) != CSA_SUCCESS)
			goto done;

		if (num_cmsattrs == 0) {
			stat = CSA_E_INVALID_PARAMETER;
			goto done;
		}

		args.cal = cal->name;
		args.pid = _DtCm_get_pid();
		args.num_attrs = num_cmsattrs;
		args.attrs = cmsattrs;
		memset((char *)&res, 0, sizeof(res));

		conn.retry = 0;
		rstat = cms_insert_entry_5(&args, &res, &conn);

		free_cmsattrs(num_cmsattrs, cmsattrs);

		if (rstat == RPC_SUCCESS) {
			if ((stat = res.stat) == CSA_SUCCESS) {
				stat = _DtCmCmsentriesToLibentries(
					&cal->entry_tbl, res.entry, entry_r);
			}

			xdr_free((xdrproc_t)xdr_cms_entry_res, (char *)&res);
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}

	} else {
 
		stat = _DtCm_table_insert(cal, &conn, num_attrs, attrs, entry_r);
	}

done:
	pthread_mutex_unlock(&ci->lock_handle);

        return(stat);
}

extern CSA_return_code
_DtCm_rpc_update_entry(
	Calendar *cal,
	_DtCm_libentry *oentry,
	CSA_uint32 num_attrs,
	CSA_attribute * attrs,
	CSA_enum scope,
	_DtCm_libentry **nentry)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;
	_DtCm_libentry		*hptr = NULL, *tptr, *cptr;
	int i;

	DP(("rpccalls.c: _DtCm_rpc_update_entry\n"));

	if (cal == NULL || oentry == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_entry_res 	res;
		cms_update_args	args;
		enum clnt_stat	rstat;
		CSA_uint32	num_cmsattrs;
		cms_attribute	*cmsattrs;

		if ((stat = csa2cmsattrs(num_attrs, attrs, NULL, &num_cmsattrs,
		    &cmsattrs, NULL)) != CSA_SUCCESS)
			goto done;

		if (num_cmsattrs == 0) {
			stat = CSA_E_INVALID_PARAMETER;
			goto done;
		}

		args.cal = cal->name;
		args.pid = _DtCm_get_pid();
		args.entry = oentry->e->key;
		args.scope = scope;
		args.num_attrs = num_cmsattrs;
		args.attrs = cmsattrs;
		memset((char *)&res, 0, sizeof(res));

		conn.retry = 0;
		rstat = cms_update_entry_5(&args, &res, &conn);

		free_cmsattrs(num_cmsattrs, cmsattrs);

		if (rstat == RPC_SUCCESS) {
			if ((stat = res.stat) == CSA_SUCCESS) {
				stat = _DtCmCmsentriesToLibentries(
					&cal->entry_tbl, res.entry, nentry);
			}

			xdr_free((xdrproc_t)xdr_cms_entry_res, (char *)&res);
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}

	} else {
 
		stat = _DtCm_table_update(cal, &conn, oentry, num_attrs, attrs,
				scope, nentry);
	}

done:
	pthread_mutex_unlock(&ci->lock_handle);

        return(stat);
}

extern CSA_return_code
_DtCm_rpc_delete_entry(
	Calendar *cal,
	_DtCm_libentry *entry,
	CSA_enum scope)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_delete_entry\n"));

	if (cal == NULL || entry == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_delete_args	args;
		enum clnt_stat	rstat;

		args.cal = cal->name;
		args.pid = _DtCm_get_pid();
		args.entry = entry->e->key;
		args.scope = scope;

		conn.retry = 0;
		rstat = cms_delete_entry_5(&args, &stat, &conn);

		if (rstat != RPC_SUCCESS)
			stat = _DtCm_clntstat_to_csastat(rstat);
	} else {
 
		stat = _DtCm_table_delete(cal, &conn, entry, scope);
	}

	pthread_mutex_unlock(&ci->lock_handle);

        return(stat);
}

extern CSA_return_code
_DtCm_rpc_list_calendar_attributes(
	Calendar	*cal,
	CSA_uint32	*number_names,
	char		***names_r)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;

	DP(("rpccalls.c: _DtCm_rpc_list_calendar_attributes\n"));

	if (cal == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}
	
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_enumerate_calendar_attr_res	res;
		char				**names;
		enum clnt_stat			rstat;
		int				i;

		*number_names = 0;
		*names_r = NULL;
		memset((char *)&res, 0, sizeof(res));

		conn.retry = 1;
		rstat = cms_enumerate_calendar_attr_5(&cal->name, &res, &conn);

		if (rstat == RPC_SUCCESS) {
			if ((stat = res.stat) == CSA_SUCCESS && res.num_names)
			{
				if (names = _DtCm_alloc_character_pointers(
				    res.num_names)) {
					for (i = 0; i < res.num_names; i++) {
						if ((names[i] = strdup(
						    res.names[i].name))
						    == NULL) {
							_DtCm_free(names);
							stat = CSA_E_INSUFFICIENT_MEMORY;
							break;
						}
					}
					if (stat == CSA_SUCCESS) {
						*number_names = res.num_names;
						*names_r = names;
					}
				} else
					stat = CSA_E_INSUFFICIENT_MEMORY;

				xdr_free((xdrproc_t)xdr_cms_enumerate_calendar_attr_res, (char *)&res);
			}
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}

	} else {
 
		stat = CSA_E_NOT_SUPPORTED;
	}

done:
	pthread_mutex_unlock(&ci->lock_handle);

        return(stat);
}

extern CSA_return_code
_DtCm_rpc_list_calendars(
	char			*location,
	CSA_uint32		*svc_version,
	CSA_uint32		*number_names,
	CSA_calendar_user	**names_r)
{
	_DtCm_Client_Info	*ci;
	CSA_return_code		stat;
	_DtCm_Connection	conn;
	CSA_uint32		vers;

	DP(("rpccalls.c: _DtCm_rpc_list_calendars\n"));

	if ((stat = _DtCm_get_rpc_handle(location, TABLEVERS,
	    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS) {
		return (stat);
	}

	vers = ci->vers_out;
	conn.ci = ci;

	if (ci->vers_out == TABLEVERS) {
		cms_list_calendars_res	res;
		enum clnt_stat		rstat;
		CSA_calendar_user	*names;
		int			i;

		*number_names = 0;
		*names_r = NULL;
		memset((char *)&res, 0, sizeof(res));

		conn.retry = 1;
		rstat = cms_list_calendars_5(NULL, &res, &conn);

		if (rstat == RPC_SUCCESS) {
			if ((stat = res.stat) == CSA_SUCCESS && res.num_names)
			{
				if (names = _DtCm_alloc_calendar_users(
				    res.num_names)) {
					for (i = 0; i < res.num_names; i++) {
						if ((names[i].calendar_address =
						    strdup(res.names[i]))
						    == NULL) {
							_DtCm_free(names);
							stat = CSA_E_INSUFFICIENT_MEMORY;
							break;
						}
					}
					if (stat == CSA_SUCCESS) {
						*number_names = res.num_names;
						*names_r = names;
					}
				} else
					stat = CSA_E_INSUFFICIENT_MEMORY;
				xdr_free((xdrproc_t)xdr_cms_list_calendars_res, (char *)&res);
			}
		} else {
			stat = _DtCm_clntstat_to_csastat(rstat);
		}

	} else {
 
		stat = CSA_E_NOT_SUPPORTED;
	}

done:
	pthread_mutex_unlock(&ci->lock_handle);

	if (stat == CSA_SUCCESS)
		*svc_version = vers;

        return(stat);
}

/*
 * rpc handle is locked by the caller
 * don't call functions that may lock the handle again
 */
extern CSA_return_code
_DtCm_do_unregistration(
	_DtCm_Connection *conn,
	char *cal,
	CSA_flags update_type)
{
	CSA_return_code	stat;

	if (conn->ci->vers_out == TABLEVERS) {
		cms_register_args	args;
		enum clnt_stat		rstat;

		args.cal = cal;
		args.update_type = update_type;
		args.prognum = _DtCm_transient2;
		args.versnum = AGENTVERS_2;
		args.procnum = update_callback;
		args.pid = _DtCm_get_pid();

		rstat = cms_unregister_5(&args, &stat, conn);

		if (rstat != RPC_SUCCESS)
			stat = _DtCm_clntstat_to_csastat(rstat);
	} else {
		stat = _DtCm_table_unregister_target(conn, cal);
	}

	if (stat == CSA_SUCCESS)
		_DtCm_remove_registration(conn->ci, cal, update_type);

	return (stat);
}

/*
 * rpc handle is locked by the caller
 * don't call functions that may lock the handle again
 */
extern CSA_return_code
_DtCm_do_registration(
	_DtCm_Connection *conn,
	char *cal,
	CSA_flags update_type)
{
	CSA_return_code	stat;

	if (conn->ci->vers_out == TABLEVERS) {
		cms_register_args	args;
		enum clnt_stat		rstat;

		args.cal = cal;
		args.update_type = update_type;
		args.prognum = _DtCm_transient2;
		args.versnum = AGENTVERS_2;
		args.procnum = update_callback;
		args.pid = _DtCm_get_pid();

		rstat = cms_register_5(&args, &stat, conn);

		if (rstat != RPC_SUCCESS)
			stat = _DtCm_clntstat_to_csastat(rstat);
	} else {

		stat = _DtCm_table_register_target(conn, cal);
	}

	if (stat == CSA_SUCCESS) {
		if ((stat = _DtCm_add_registration(conn->ci, cal, update_type))
		    != CSA_SUCCESS) {
			(void) _DtCm_do_unregistration(conn, cal, update_type);
		}
	}

	return (stat);
}

extern CSA_return_code
_DtCm_rpc_get_server_version(char *host, int *version)
{
	CSA_return_code stat;
	_DtCm_Client_Info *ci;

	if (host == NULL || version == NULL) {
		return (CSA_E_INVALID_PARAMETER);
	}

	if ((stat = _DtCm_get_rpc_handle(host, TABLEVERS,
	    _DtCM_INITIAL_TIMEOUT, &ci)) == CSA_SUCCESS) {
		*version = ci->vers_out;
		pthread_mutex_unlock(&ci->lock_handle);
	}

	return (stat);
}

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

/*
 * Newnum is initialized to 0 and newattrs to NULL when num_attrs == 0
 */
static CSA_return_code
csa2cmsattrs(
	CSA_uint32	num_attrs,
	CSA_attribute	*csaattrs,
	CSA_enum	*ops,
	CSA_uint32	*newnum,
	cms_attribute	**newattrs,
	CSA_enum	**newops)
{
	CSA_return_code	stat = CSA_SUCCESS;
	cms_attribute	*cmsattrs;
	CSA_enum	*ops_r;
	CSA_reminder	*rptr1, *rptr2;
	int		i,j;

	*newnum = 0;
	*newattrs = NULL;
	if (newops) *newops = NULL;

	if (num_attrs == 0)
		return CSA_SUCCESS;

	if ((cmsattrs = calloc(1, sizeof(cms_attribute)*num_attrs)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (newops && (ops_r = malloc(sizeof(CSA_enum)*num_attrs)) == NULL) {
		free(cmsattrs);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	for (i = 0, j = 0; i < num_attrs && stat == CSA_SUCCESS; i++) {
		if (csaattrs[i].name == NULL)
			continue;

		if (newops)
			ops_r[j] = (ops ? ops[i] : CSA_MATCH_EQUAL_TO);

		cmsattrs[j].name.name = csaattrs[i].name;
		if (csaattrs[i].value == NULL) {
			j++;
			continue;
		}

		if ((cmsattrs[j].value = (cms_attribute_value *)malloc(
		    sizeof(cms_attribute_value))) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		cmsattrs[j].value->type = csaattrs[i].value->type;

		switch (csaattrs[i].value->type) {
		case CSA_VALUE_ACCESS_LIST:
			stat = _DtCm_csa2cms_access_list(
				csaattrs[i].value->item.access_list_value,
				&cmsattrs[j].value->item.access_list_value);
			break;
		case CSA_VALUE_CALENDAR_USER:
			if (csaattrs[i].value->item.calendar_user_value &&
			    csaattrs[i].value->item.calendar_user_value->\
			    user_name)
			{
				cmsattrs[j].value->item.calendar_user_value =
			    		csaattrs[i].value->item.\
					calendar_user_value->user_name;
			} else {
				cmsattrs[j].value->item.calendar_user_value =
					nullstr;
			}
			break;
		case CSA_VALUE_STRING:
		case CSA_VALUE_DATE_TIME:
		case CSA_VALUE_DATE_TIME_RANGE:
		case CSA_VALUE_TIME_DURATION:
			if (csaattrs[i].value->item.string_value) {
				cmsattrs[j].value->item.string_value =
					csaattrs[i].value->item.string_value;
			} else {
				cmsattrs[j].value->item.string_value = nullstr;
			}
			break;
		case CSA_VALUE_REMINDER:
			if ((rptr1 = csaattrs[i].value->item.reminder_value) &&
			    (rptr2 = (CSA_reminder *)malloc(sizeof(CSA_reminder))))
			{
				rptr2->lead_time = rptr1->lead_time ?
						    rptr1->lead_time : nullstr;
				rptr2->snooze_time = rptr1->snooze_time ?
						     rptr1->snooze_time:nullstr;
				rptr2->repeat_count = rptr1->repeat_count;
				rptr2->reminder_data = rptr1->reminder_data;
				cmsattrs[j].value->item.reminder_value = rptr2;
			} else if (rptr1 && rptr2 == NULL)
				stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		default:
			/* all other value types uses the same type
			 * of data, so we just copy the value/address of
			 * of the data
			 */
			cmsattrs[j].value->item.sint32_value =
				csaattrs[i].value->item.sint32_value;
		}
		j++;
	}

	if (stat != CSA_SUCCESS) {

		free_cmsattrs(num_attrs, cmsattrs);
		if (newops) free(ops_r);

	} else if (j > 0) {
		*newnum = j;
		*newattrs = cmsattrs;
		if (newops) *newops = ops_r;
	} else {
		free(cmsattrs);
		if (newops) free(ops_r);
	}

	return (stat);
}

static void
free_cmsattrs(CSA_uint32 num_attrs, cms_attribute *attrs)
{
	int i;

	for (i = 0; i < num_attrs; i++) {
		if (attrs[i].value == NULL)
			continue;

		switch (attrs[i].value->type) {
		case CSA_VALUE_ACCESS_LIST:
			_DtCm_free_cms_access_entry(
				attrs[i].value->item.access_list_value);
			break;
		case CSA_VALUE_REMINDER:
			if (attrs[i].value->item.reminder_value)
				free(attrs[i].value->item.reminder_value);
			break;
		}

		free(attrs[i].value);
	}
	free (attrs);
}

/*
 * Newnum is initialized to 0 and newattrs to NULL when num_attrs == 0
 */
static CSA_return_code
csa2cmsnames(
	CSA_uint32	num_names,
	char		**names,
	CSA_uint32	*newnum,
	cms_attr_name	**newnames)
{
	cms_attr_name	*cmsnames;
	int		i,j;

	*newnum = 0;
	*newnames = NULL;

	if (num_names == 0)
		return (CSA_SUCCESS);

	if ((cmsnames = calloc(1, sizeof(cms_attr_name)*num_names)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0, j = 0; i < num_names; i++) {
		if (names[i] == NULL)
			continue;
		else {
			cmsnames[j].name = names[i];
			j++;
		}
	}

	if (j > 0) {
		*newnum = j;
		*newnames = cmsnames;
	} else
		free(cmsnames);

	return (CSA_SUCCESS);
}

/*
 * It is an old server and so we need to determine the user's access
 * w.r.t. the calendar ourselves.
 * An rpc call (table_set_access) is made to test whether the user
 * is the owner of the calendar (only owner can make this call successfully).
 * Hopefully, this does not happen very often.
 * If the user is not the owner, we'll go through the access list
 * to get the user's access.
 */
static CSA_return_code
_GetV4UserAccess(Calendar *cal, _DtCm_Connection *conn, cms_access_entry *alist)
{
	CSA_return_code	stat = CSA_SUCCESS;
	char		buf[BUFSIZ];
	char		*ptr;
	CSA_uint32	worldAccess=0, userAccess=0, userAtAccess=0;
	boolean_t	foundUser = _B_FALSE, foundUserAt = _B_FALSE;
	CSA_access_rights	*csalist;

	/* test whether the user is the owner */
	if ((stat = _DtCm_cms2csa_access_list(alist, &csalist)) == CSA_SUCCESS)
	{
		stat = _DtCm_table_set_access(cal, conn, csalist);
		_DtCm_free_csa_access_list(csalist);

		if (stat == CSA_SUCCESS) {
			cal->access = CSA_OWNER_RIGHTS;
			return (CSA_SUCCESS);
		} else if (stat != CSA_E_NO_AUTHORITY) {
			return (stat);
		}
	} else
		return (stat);

	sprintf(buf, "%s@%s", _DtCmGetUserName(), _DtCmGetHostAtDomain());
	for (; alist != NULL; alist = alist->next) {
		if (strcmp(alist->user, "world") == 0)
			worldAccess = alist->rights;
		else if (_DtCmIsSameUser(buf, alist->user) == _B_TRUE) {
			foundUser = _B_TRUE;
			if (ptr = strchr(alist->user, '@')) {
				foundUserAt = _B_TRUE;
				userAtAccess = alist->rights;
			} else
				userAccess = alist->rights;
		}
	}

	if (foundUser == _B_TRUE)
		cal->access = foundUserAt ? userAtAccess : userAccess;
	else
		cal->access = worldAccess;

	return (CSA_SUCCESS);
}

static CSA_return_code
_get_entry_details(
	Calendar		*cal,
	_DtCm_Connection	conn,
	cms_reminder_ref	*cmsrems,
	CSA_uint32		*num_rems,
	CSA_reminder_reference	**csarems)
{
	CSA_return_code		stat;
	cms_get_entry_attr_args	args;
	cms_get_entry_attr_res	res;
	cms_reminder_ref	*rptr;
	cms_key			*keys;
	enum clnt_stat		rstat;
	int			i, count;
	_DtCm_Client_Info	*oldci, *ci = NULL;

	DP(("rpccalls.c: _get_entry_details\n"));

	for (count = 0, rptr = cmsrems; rptr != NULL; rptr = rptr->next)
		count++;

	if ((keys = (cms_key *)calloc(1, sizeof(cms_key) * count)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0, rptr = cmsrems; rptr != NULL; rptr = rptr->next)
		keys[i++] = rptr->key;

	args.cal = cal->name;
	args.num_keys = count;
	args.keys = keys;
	args.num_names = 0;
	args.names = NULL;
	memset((char *)&res, 0, sizeof(res));

	conn.retry = 1;
	rstat = cms_get_entry_attr_5(&args, &res, &conn);

	if (rstat == RPC_SUCCESS && res.stat == CSA_E_NOT_SUPPORTED) {
		Uid_4	*uids;
		Appt_4	*appt;

		/* server does not support cms_get_entry_attrs_5 on data
		 * version 4 calendars, need to get a rpc version 4 connection
		 * and call table_lookup()
		 */
		if ((stat = _DtCm_get_rpc_handle(cal->location, TABLEVERS_4,
		    _DtCM_LONG_TIMEOUT, &ci)) != CSA_SUCCESS)
			goto done;

		oldci = conn.ci;
		conn.ci = ci;

		if ((stat = _DtCm_cmskey_to_uid4(count, keys, &uids))
		    != CSA_SUCCESS)
			goto done;

		if ((stat = _DtCm_table_lookup(cal->name, &conn, uids, &appt))
		    == CSA_SUCCESS) {
			_DtCm_free_uid4(uids);

			stat = _DtCm_convert_appt_to_reminder_detail(cal->name,
				cmsrems, appt, num_rems, csarems); 

			_DtCm_free_appt4(appt);
		}

	} else if (rstat == RPC_SUCCESS) {
		if ((stat = res.stat) == CSA_SUCCESS) {
			stat = _DtCm_cms2csa_reminder_detail(cal, cmsrems,
				res.entries, num_rems, csarems);
		}
		
		xdr_free(xdr_cms_get_entry_attr_res, (char *)&res);
	} else {
		stat = _DtCm_clntstat_to_csastat(rstat);
	}

done:
	if (ci != NULL) {
		pthread_mutex_unlock(&ci->lock_handle);
		conn.ci = oldci;
	}
	free(keys);
        return(stat);
}

