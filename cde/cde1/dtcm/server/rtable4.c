/* $XConsortium: rtable4.c /main/cde1_maint/3 1995/10/10 13:36:18 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)rtable4.c	1.41 97/06/17 Sun Microsystems, Inc."

/*
 * version 4 of calendar manager rpc protocol functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rtable4.h"
#include <sys/param.h>
#include <sys/time.h>
#include <signal.h>
#include <rpc/rpc.h>
#include <values.h>
#include <string.h>
#include <pwd.h>
#ifdef SUNOS
#include <netdir.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#endif
#ifdef MT
#include <pthread.h>
#endif
#include "cm.h"
#include "access.h"
#include "laccess.h"
#include "callback.h"
#include "appt4.h"		
#include "log.h"
#include "tree.h"
#include "list.h"
#include "cmscalendar.h"
#include "v4ops.h"
#include "v5ops.h"
#include "reminder.h"
#include "repeat.h"
#include "utility.h"
#include "lutil.h"
#include "rpcextras.h"
#include "rtable4_tbl.i"
#include "lookup.h"
#include "cmsdata.h"
#include "attr.h"
#include "convert5-4.h"
#include "convert4-5.h"
#include "cmsconvert.h"
#include "misc.h"
#include "insert.h"
#include "delete.h"
#include "update.h"


extern	int	debug;

/*****************************************************************************
 * forward declaration of static functions used within the file
 *****************************************************************************/

static Access_Status_4 csastat2accessstat(CSA_return_code stat);

static Registration_Status_4 csastat2regstat(CSA_return_code stat);

static Table_Status_4 csastat2tablestat(CSA_return_code stat);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

/*
 * supports both data format
 */
extern bool_t
_DtCm_rtable_lookup_4_svc (Table_Args_4 *args, Table_Res_4 *res, struct svc_req *svcrq)
{
	CSA_return_code	stat;
	Appt_4		*p_appt;
	Appt_4		*h = NULL;
	_DtCmsCalendar	*cal;
	Uid_4		*p_keys;
	Id_4		*key;
	_DtCmsSender	*user;
	CSA_flags	access;
	cms_entry	*entries;
	cms_key		cmskey;
	time_t		tmptick = 0;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_lookup_4_svc called\n");

	res->status = access_other_4;
	res->res.tag = AP_4;
	res->res.Table_Res_List_4_u.a = NULL;
	if ((p_keys = args->args.Args_4_u.key) == NULL)
		return (1);

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		return (1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_VIEW_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_BROWSE_ACCESS(access))) {
		res->status = access_failed_4;
		_DtCmsUnlockCalendar(cal);
		return (1);
	} else {
		res->status = access_ok_4;
	}

	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		while (p_keys != NULL) {
			cmskey.time = p_keys->appt_id.tick;
			cmskey.id = p_keys->appt_id.key;

			if ((stat = _DtCmsGetEntryAttrByKey(cal, user->name,
			    access, cmskey, 0, NULL, &entries, NULL))
			    == CSA_SUCCESS) {

				if ((stat = _DtCmsCmsentriesToAppt4ForClient(
				    entries, &p_appt)) == CSA_SUCCESS) {
					/* link to appt list */
					h = _AddApptInOrder(h, p_appt);
				}
				_DtCm_free_cms_entries(entries);
			}

			if (stat != CSA_SUCCESS) {
				res->status = csastat2accessstat(stat);
				if (h) {
					_DtCm_free_appt4(h);
					h = NULL;
				}
				break;
			}
			p_keys = p_keys->next;
		}

		res->res.Table_Res_List_4_u.a = h;
		_DtCmsUnlockCalendar(cal);
		return (1);
	}

	/* do lookup on old format calendar */
	while (p_keys != NULL)
	{
		key = &p_keys->appt_id;

		if (debug) {
		  char cbuf[26];

		  ctime_r(&key->tick, cbuf);
		  fprintf(stderr, "_DtCm_rtable_lookup_4_svc at (key %d)%s\n", key->key, cbuf);
		}

		p_appt = (Appt_4 *)rb_lookup(APPT_TREE(cal), (caddr_t)key);
		if (p_appt == NULL) {
			if ((p_appt = (Appt_4 *)hc_lookup(REPT_LIST(cal),
			    (caddr_t)key)) != NULL) {
				if (!_DtCms_in_repeater(key, p_appt, _B_FALSE))
					p_appt = NULL;
			}
		}

		if (p_appt != NULL) {
			if (p_appt->appt_id.tick != key->tick) {
				tmptick = p_appt->appt_id.tick;
				p_appt->appt_id.tick = key->tick;
			}

			stat = _AddToLinkedAppts(p_appt, user->name, access,
					(caddr_t *)&h);

			if (tmptick) p_appt->appt_id.tick = tmptick;

			if (stat != CSA_SUCCESS) {
				if (h) {
					_DtCm_free_appt4(h);
					h = NULL;
				}
				res->status = csastat2accessstat(stat);
			}
		}

		p_keys = p_keys->next;
	}

	res->res.Table_Res_List_4_u.a = h;
	_DtCmsUnlockCalendar(cal);
	return (1);
}

/*
 * supports old data format only
 */
extern bool_t
_DtCm_rtable_lookup_next_larger_4_svc(Table_Args_4 *args, Table_Res_4 *res, struct svc_req *svcrq)
{
	if (debug)
		fprintf(stderr, "_DtCm_rtable_lookup_next_larger_4_svc called\n");

	res->status = access_notsupported_4;
	return (1);
}

/*
 * supports old data format only
 */
extern bool_t
_DtCm_rtable_lookup_next_smaller_4_svc(
	Table_Args_4	*args,
	Table_Res_4 *res,
	struct svc_req	*svcrq)
{
	if (debug)
		fprintf(stderr, "_DtCm_rtable_lookup_next_smaller_4_svc called\n");

	res->status = access_notsupported_4;
	return (1);
}

/*
 * supports both data format
 */
extern bool_t
_DtCm_rtable_lookup_range_4_svc(Table_Args_4 *args, Table_Res_4 *res, struct svc_req *svcrq)
{
	CSA_return_code	stat;
	_DtCmsCalendar	*cal;
	_DtCmsSender	*user;
	CSA_flags	access;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_lookup_range_4_svc called\n");

	res->res.tag = AP_4;
	res->res.Table_Res_List_4_u.a = NULL;

	res->status = access_other_4;
	if (args->args.Args_4_u.range == NULL)
		return (1);

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		return (1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_VIEW_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_BROWSE_ACCESS(access))) {
		res->status = access_failed_4;
		_DtCmsUnlockCalendar(cal);
		return (1);
	}

	stat = _DtCmsLookupRangeV4(cal, user->name, access,
		args->args.Args_4_u.range,
		_B_TRUE, 0, 0, NULL, 0, NULL, NULL,
		&res->res.Table_Res_List_4_u.a, NULL);

	res->status = csastat2accessstat(stat);
	_DtCmsUnlockCalendar(cal);
	return (1);
}

/*
 * supports both data format
 */
extern bool_t
_DtCm_rtable_abbreviated_lookup_range_4_svc(
	Table_Args_4	*args,
	Table_Res_4 *res,
	struct svc_req	*svcrq)
{
	CSA_return_code	stat;
	_DtCmsCalendar	*cal;
	_DtCmsSender	*user;
	CSA_flags	access;

	if (debug)
		fprintf(stderr,
			"_DtCm_rtable_abbreviated_lookup_range_4_svc called\n");

	res->res.tag = AB_4;
	res->res.Table_Res_List_4_u.b = NULL;

	res->status = access_other_4;
	if (args->args.Args_4_u.range == NULL)
		return (1);

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		return (1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_VIEW_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_BROWSE_ACCESS(access))) {
		res->status = access_failed_4;
		_DtCmsUnlockCalendar(cal);
		return (1);
	}

	stat = _DtCmsLookupRangeV4(cal, user->name, access,
		args->args.Args_4_u.range,
		_B_TRUE, 0, 0, NULL, 0, NULL, NULL, NULL,
		&res->res.Table_Res_List_4_u.b);

	res->status = csastat2accessstat(stat);
	_DtCmsUnlockCalendar(cal);
	return (1);
}

extern bool_t
_DtCm_rtable_insert_4_svc(Table_Args_4 *args, Table_Res_4 *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	CSA_return_code		stat;
	_DtCmsSender		*user;
	CSA_flags		access;
	Appt_4			*a, *ap, *appt, *prev;
	cms_entry		*entry;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_insert_4_svc called\n");

	res->status = access_other_4;
	res->res.tag = AP_4;
	res->res.Table_Res_List_4_u.a = NULL;

	/* check arguments */
	if (args->target == NULL)
		return (1);
	if (args->args.Args_4_u.appt == NULL)
		return (1);

	/* do some sanity checks before inserting : check appt data */
	for (appt = args->args.Args_4_u.appt; appt != NULL; appt = appt->next)
	{
		/* ntimes should be 0 or positive */
		if (appt->ntimes < 0 ||
		    (appt->period.period > single_4 && appt->ntimes == 0))
			return(1);

		/* period beyond daysOfWeek is not supported */
		if (appt->period.period > daysOfWeek_4) {
			res->status = access_notsupported_4;
			return(1);
		}

		/* if weekmask of daysOfWeek appt is set incorrectly, return */
		if (appt->period.period == daysOfWeek_4 &&
			(appt->period.nth == 0 || appt->period.nth > 127))
			return(1);
	}

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		return (1);
	}

	if ((cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_WRITE_ACCESS(access)) ||
	    (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_INSERT_ACCESS(access))) {
		res->status = access_failed_4;
		_DtCmsUnlockCalendar(cal);
		return (1);
	}

	/* Verify that the callog file has not been modified by
	 * something other than rpc.cmsd.
	 */
	if ((stat = _DtCmsVerifyLogFileInfo(cal->calendar, cal->modtime, cal->blksize)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* make copy of the appointments */
	/* this copy is used in the result and will be freed
	 * when this routine is called again
	 */
	if ((appt = _DtCm_copy_appt4(args->args.Args_4_u.appt)) == NULL) {
	        _DtCmsUnlockCalendar(cal);
		/* memory problem */
		return (1);
	}

	ap = appt;
	prev = NULL;
	while (appt != NULL) {

		/*
		 * we used to calculate the correct start day,
		 * but we should return an error instead
		 */
		_DtCms_adjust_appt_startdate(appt);

		if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
			/* convert to attributes */
			if (_DtCmsAppt4ToCmsentry(args->target, appt, &entry,
			    _B_TRUE))
				goto insert_error;

			/* null out readonly attributes */
			_DtCm_free_cms_attribute_value(entry->attrs\
				[CSA_ENTRY_ATTR_ORGANIZER_I].value);
			entry->attrs[CSA_ENTRY_ATTR_ORGANIZER_I].value = NULL;
			_DtCm_free_cms_attribute_value(entry->attrs\
				[CSA_ENTRY_ATTR_REFERENCE_IDENTIFIER_I].value);
			entry->attrs[CSA_ENTRY_ATTR_REFERENCE_IDENTIFIER_I].\
				value = NULL;

			if (_DtCmsCheckInitialAttributes(entry)) {
				_DtCm_free_cms_entry(entry);
				goto insert_error;
			}

			/* set organizer */
			if (_DtCm_set_string_attrval(user->name, &entry->attrs\
			    [CSA_ENTRY_ATTR_ORGANIZER_I].value,
	    		    CSA_VALUE_CALENDAR_USER) != CSA_SUCCESS) {
				_DtCm_free_cms_entry(entry);
				goto insert_error;
			}

			/* insert entry and log it */
			if (_DtCmsInsertEntryAndLog(cal, entry)) {
				_DtCm_free_cms_entry(entry);
				goto insert_error;
			}

			appt->appt_id.key = entry->key.id;
			_DtCm_free_cms_entry(entry);

		} else {
			if ((a = _DtCm_copy_one_appt4(appt)) == NULL)
				goto insert_error;

			/* We don't trust the author field; we set our own. */
			free(a->author);
			if ((a->author = strdup(user->name)) == NULL) {
				_DtCm_free_appt4(a);
				goto insert_error;
			}

			/* Note, the key in appt will be set if its value is 0. */
			if ((stat = _DtCmsInsertApptAndLog(cal, a)) != CSA_SUCCESS) {
				res->status = csastat2accessstat(stat);
				goto insert_error;
			}

			/* get the new key */
			appt->appt_id.key = a->appt_id.key;
		}

		prev = appt;
		appt = appt->next;
	}

	cal->modified = _B_TRUE;

	/* Write has been successful.  Update the modtime and
	 * blksize info, since the callog file has been updated.
	 */
	_DtCmsUpdateLogFileInfo(cal->calendar, &(cal->modtime), 
		&(cal->blksize));


	/* do callbacks */
	cal->rlist = _DtCmsDoV1Callback(cal->rlist, user->name, args->pid, ap);

	for (appt = ap; appt != NULL; appt = appt->next) {
		cal->rlist = _DtCmsDoInsertEntryCallback(cal->rlist,
				cal->calendar, user->name, appt->appt_id.key,
				args->pid);
	}

	res->status = access_ok_4;
	res->res.Table_Res_List_4_u.a = ap;

	_DtCmsUnlockCalendar(cal);
	return (1);

insert_error:
	if (prev != NULL) {
		cal->modified = _B_TRUE;
		/* some appts were inserted successfully */
		res->status = access_partial_4;
		prev->next = NULL;
		res->res.Table_Res_List_4_u.a = ap;

		/* do callback */
		cal->rlist = _DtCmsDoV1Callback(cal->rlist, user->name,
				args->pid, ap);
		for (appt = ap; appt != NULL; appt = appt->next) {
			cal->rlist = _DtCmsDoInsertEntryCallback(cal->rlist,
					cal->calendar, user->name,
					appt->appt_id.key, args->pid);
		}

	} else {
		/* first appt in bunch that failed */
		res->status = csastat2accessstat(stat);
	}
	_DtCm_free_appt4(appt);

	_DtCmsUnlockCalendar(cal);
	return (1);
}

extern bool_t
_DtCm_rtable_delete_4_svc(Table_Args_4 *args, Table_Res_4 *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	CSA_return_code		stat;
	_DtCmsSender		*user;
	CSA_flags		access;
	Uidopt_4		*p_keys;
	Appt_4			*h = NULL;
	Appt_4			*a;
	int			d, n, nf;
	cms_key			key;
	cms_entry		*entry;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_delete_4_svc called\n");

	res->status = access_other_4;
	res->res.tag = AP_4;
	res->res.Table_Res_List_4_u.a = NULL;

	/* check arguments */
	if (args->target == NULL)
		return (1);
	if ((p_keys = args->args.Args_4_u.uidopt) == NULL)
		return (1);

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		return (1);
	}

	/* access right is also checked when the appt is being deleted
	 * to make sure the sender can delete it
	 */
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_CHANGE_ACCESS(access)) {
		res->status = access_failed_4;
		_DtCmsUnlockCalendar(cal);
		return (1);
	}

	/* Verify that the callog file has not been modified by
	 * something other than rpc.cmsd.
	 */
	if ((stat = _DtCmsVerifyLogFileInfo(cal->calendar, cal->modtime, cal->blksize)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	nf = d = n = 0;
	while (p_keys != NULL) {
		n++;
		if (debug) {
		  char cbuf[26];

		  ctime_r(&p_keys->appt_id.tick, cbuf);
		  fprintf (stderr, "Delete: (key %d)%s\n",
			      p_keys->appt_id.key,
			      cbuf);
		}

		if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
			key.time = p_keys->appt_id.tick;
			key.id = p_keys->appt_id.key;

			if (p_keys->option == do_all_4)
				stat = _DtCmsDeleteEntryAndLog(cal, user->name,
					access, &key, &entry);
			else
				stat = _DtCmsDeleteInstancesAndLog(cal,
					user->name, access, &key,
					(p_keys->option == CSA_SCOPE_ONE ?
					CSA_SCOPE_ONE : CSA_SCOPE_FORWARD),
					NULL, &entry); 

			if (stat == CSA_SUCCESS)
				stat = _DtCm_cms_entry_to_appt4(entry, &a);

			_DtCm_free_cms_entry(entry);

		} else {

			/* single or all in a repeating series */
			if (p_keys->option == do_all_4)
				stat = _DtCmsDeleteApptAndLog(cal, user->name,
					access, &p_keys->appt_id, &a);
			else {
				stat = _DtCmsDeleteApptInstancesAndLog(cal,
					user->name, access, &p_keys->appt_id,
					p_keys->option, NULL, &a);
			}
		}

		if (stat == CSA_SUCCESS) {

			/* Write has been successful.  Update the modtime and
			 * blksize info, since the callog file has been updated.
			 */
			_DtCmsUpdateLogFileInfo(cal->calendar, &(cal->modtime),
				&(cal->blksize));

			if (p_keys->option != do_all_4)
				APPT_TICK(a) = p_keys->appt_id.tick;

			d++;
			a->next = h;
			h = a;
		} else if (stat == CSA_X_DT_E_ENTRY_NOT_FOUND)
			nf++;

		p_keys = p_keys->next;
	}

	if (h != NULL) {
		cal->modified = _B_TRUE;

		/* do callback */
		cal->rlist = _DtCmsDoV1Callback(cal->rlist, user->name, args->pid, h);
		for (a = h, p_keys = args->args.Args_4_u.uidopt;
		    a != NULL; a = a->next) {
			while (a->appt_id.key != p_keys->appt_id.key)
				p_keys = p_keys->next;

			cal->rlist = _DtCmsDoDeleteEntryCallback(cal->rlist,
					cal->calendar, user->name,
					a->appt_id.key, p_keys->option,
					(p_keys->option == do_all_4 ?
					a->appt_id.tick : p_keys->appt_id.tick),
					args->pid);
		}
	}

	if (d == 0) {
		if (stat != 0)
			res->status = csastat2accessstat(stat);
		else
			res->status = (nf < n) ? access_failed_4 : access_ok_4;
	} else if (d < n)
		res->status = access_partial_4;
	else
		res->status = access_ok_4;

	res->res.Table_Res_List_4_u.a = h;

	_DtCmsUnlockCalendar(cal);
	return (1);
}

extern bool_t
_DtCm_rtable_delete_instance_4_svc(Table_Args_4 *args, Table_Res_4 *res, struct svc_req *svcrq)
{
	if (debug)
		fprintf(stderr, "_DtCm_rtable_delete_instance_4_svc called\n");

	res->status = access_notsupported_4;
	res->res.tag = AP_4;
	res->res.Table_Res_List_4_u.a = NULL;

	return(1);
}

extern bool_t
_DtCm_rtable_change_4_svc(Table_Args_4 *args, Table_Res_4 *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	CSA_return_code		stat;
	_DtCmsSender		*user;
	CSA_flags		access;
	Id_4			*p_key;
	Appt_4			*newa, *olda;
	Appt_4			tmpappt;
	Options_4		option;
	cms_entry		*entry, *oldentry, *newentry = NULL;
	cms_key			key;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_change_4_svc called\n");

	/* check arguments */
	if (args->target == NULL)
		return (1);
	if ((p_key = args->args.Args_4_u.apptid.oid) == NULL)
		return (1);
	if ((newa = args->args.Args_4_u.apptid.new_appt) == NULL)
		return (1);

	/* ntimes should be 0 or positive */
	if (newa->ntimes < 0 ||
	    (newa->period.period > single_4 && newa->ntimes == 0))
		return(1);

	/* period beyond daysOfWeek is not supported */
	if (newa->period.period > daysOfWeek_4) {
		res->status = access_notsupported_4;
		return(1);
	}

	/* if weekmask of daysOfWeek appt is not set correctly, return */
	if (newa->period.period == daysOfWeek_4 &&
	    (newa->period.nth == 0 || newa->period.nth > 127))
		return(1);

	option = args->args.Args_4_u.apptid.option;
	if (option < do_all_4 || option > do_forward_4)
		return (1); 

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		return (1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_CHANGE_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_WRITE_ACCESS(access))) {
		res->status = access_failed_4;
		_DtCmsUnlockCalendar(cal);
		return (1);
	}

	/* Verify that the callog file has not been modified by
	 * something other than rpc.cmsd.
	 */
	if ((stat = _DtCmsVerifyLogFileInfo(cal->calendar, cal->modtime, cal->blksize)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		return(1);
	}

	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		/* convert to attributes */
		if ((stat = _DtCmsAppt4ToCmsentry(args->target, newa, &entry,
		    _B_TRUE)) == CSA_SUCCESS) {

			key.time = p_key->tick;
			key.id = p_key->key;

			/* null out readonly attributes */
			_DtCm_free_cms_attributes(1,
				&entry->attrs[CSA_ENTRY_ATTR_ORGANIZER_I]);
			_DtCm_free_cms_attributes(1, &entry->attrs\
				[CSA_ENTRY_ATTR_REFERENCE_IDENTIFIER_I]);
			_DtCm_free_cms_attributes(1,
				&entry->attrs[CSA_ENTRY_ATTR_TYPE_I]);

			/* update entry */
			if (option == do_all_4)
				stat = _DtCmsUpdateEntry(cal, user->name,
					access, &key, entry->num_attrs,
					&entry->attrs[1], &oldentry, NULL);
			else
				stat = _DtCmsUpdateInstances(cal, user->name,
					access, &key, (option == do_one_4 ?
					CSA_SCOPE_ONE : CSA_SCOPE_FORWARD),
					entry->num_attrs, &entry->attrs[1],
					&oldentry, &newentry);

			_DtCm_free_cms_entry(entry);
		}
	} else {

		if (option == do_all_4)
			stat = _DtCmsChangeAll(cal, user->name, access,
				p_key, newa, &olda);
		else
			stat = _DtCmsChangeSome(cal, user->name,
				access, p_key, newa, option, &olda);
	}

	if (stat == CSA_SUCCESS) {
		if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
			tmpappt.appt_id.tick = oldentry->key.time;
			tmpappt.appt_id.key = oldentry->key.id;
			olda = &tmpappt;
			_DtCm_free_cms_entry(oldentry);
			if (newentry) {
				newa->appt_id.key = newentry->key.id;
				_DtCm_free_cms_entry(newentry);
			}
		}

		/* If the date/time is changed, we do a callback
		 * with the old and new appointments.  Otherwise,
		 * we only do callback with the new appointmnt.
		 */
		if (APPT_TICK(newa) == APPT_TICK(olda)) {
			cal->rlist = _DtCmsDoV1Callback(cal->rlist, user->name,
					args->pid, newa);
		} else {
			olda->next = newa;
			cal->rlist = _DtCmsDoV1Callback(cal->rlist, user->name,
					args->pid, olda);
			olda->next = NULL;
		}

		cal->rlist = _DtCmsDoUpdateEntryCallback(cal->rlist,
				cal->calendar, user->name,
				(newa->appt_id.key == olda->appt_id.key ?
				0 : newa->appt_id.key),
				olda->appt_id.key, option, (option == do_all_4 ?
				olda->appt_id.tick : p_key->tick),
				args->pid);

		cal->modified = _B_TRUE;

		/* Return the new appointment. */
		res->res.Table_Res_List_4_u.a = newa;
		res->status = access_ok_4;

		if (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION)
			_DtCm_free_appt4(olda);

		/* Write has been successful.  Update the modtime and
		 * blksize info, since the callog file has been updated.
		 */
		_DtCmsUpdateLogFileInfo(cal->calendar, &(cal->modtime),
			&(cal->blksize));
	} else
		res->status = csastat2accessstat(stat);

	_DtCmsUnlockCalendar(cal);
	return (1);
}

extern bool_t
_DtCm_rtable_change_instance_4_svc(Table_Args_4 *args, Table_Res_4 *res, struct svc_req *svcrq)
{
	if (debug)
		fprintf(stderr, "_DtCm_rtable_change_instance_4_svc called\n");

	res->status = access_notsupported_4;
	res->res.tag = AP_4;
	res->res.Table_Res_List_4_u.a = NULL;

	return(1);
}

extern bool_t
_DtCm_rtable_lookup_next_reminder_4_svc(
	Table_Args_4	*args,
	Table_Res_4 *res,
	struct svc_req	*svcrq)
{
	CSA_return_code		stat;
	_DtCmsSender		*user;
	CSA_flags		access;
	_DtCmsCalendar		*cal;
	time_t			tick;
	cms_reminder_ref	*rems;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_lookup_next_reminder_4_svc called\n");

	res->status = access_other_4;
	res->res.tag = RM_4;
	res->res.Table_Res_List_4_u.r = NULL;

	if (args->target == NULL)
		return (1);

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		return (1);
	}
	
	/* only user with owner rights can lookup reminders */
	if (!(access & CSA_OWNER_RIGHTS)) {
		res->status = access_failed_4;
		_DtCmsUnlockCalendar(cal);
		return (1);
	}

	tick = args->args.Args_4_u.tick;

	if (debug) {
	  char cbuf[26];

	  ctime_r(&tick, cbuf);
	  fprintf(stderr, "Next reminder after %s", cbuf);
	}

	if (cal->fversion > 1) {
		if ((stat = _DtCmsLookupReminder(cal->remq, tick, 0, NULL,
		    &rems)) == CSA_SUCCESS) {
			stat = _DtCmsReminderRefToReminder(rems,
			    &res->res.Table_Res_List_4_u.r);
			_DtCmsFreeReminderRef(rems);
		}
	} else {

		stat = _DtCmsGetV4Reminders(cal, tick,
			&res->res.Table_Res_List_4_u.r, NULL);
	}

	res->status =  csastat2accessstat(stat);
	_DtCmsUnlockCalendar(cal);
	return (1);
}

extern bool_t
_DtCm_rtable_check_4_svc(Table_Args_4 *args, Table_Status_4 *res, struct svc_req *svcrq)
{
	if (debug)
		fprintf (stderr, "_DtCm_rtable_check_4_svc called\n");

	*res = tbl_notsupported_4;
	return (1);
}

extern bool_t
_DtCm_rtable_flush_table_4_svc(Table_Args_4 *args, Table_Status_4 *res, struct svc_req *svcrq)
{
	if (debug)
		fprintf (stderr, "_DtCm_rtable_flush_table_4_svc called\n");

	*res = tbl_notsupported_4;
	return (1);
}

extern bool_t
_DtCm_rtable_size_4_svc(Table_Args_4 *args, int *res, struct svc_req *svcrq)
{
	_DtCmsCalendar	*cal;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_size_4_svc called\n");

	*res = 0;
	if (_DtCmsGetCalendarByName(args->target, &cal) != CSA_SUCCESS)
		return(1);

	*res = rb_size (APPT_TREE(cal)) + hc_size (REPT_LIST(cal));

	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern bool_t
_DtCm_register_callback_4_svc(Registration_4 *r, Registration_Status_4 *res, struct svc_req *svcrq)
{
	CSA_return_code stat;
	_DtCmsSender *user;
	_DtCmsCalendar *cal = NULL;
        _DtCmsRegistrationInfo *copy=NULL;

	if (debug)
		fprintf(stderr, "_DtCm_register_callback_4_svc called\n");

	/* Error */
        if (r->target == NULL) {
		*res = failed_4;
                return(1);
	}

	if ((stat = _DtCmsGetClientInfo(svcrq, &user)) != CSA_SUCCESS) {
		*res = csastat2regstat(stat);
		return (1);
	}

	/* check if the target exists */
	if ((stat = _DtCmsGetCalendarByName(r->target, &cal))
	    != CSA_SUCCESS) {
		*res = csastat2regstat(stat);
		return (1);
	}

	/* Check for duplicate registrations */
	if (_DtCmsGetRegistration(&(cal->rlist), user->name, r->prognum,
	    r->versnum, r->procnum, r->pid) == NULL) {
		/* not registered */

        	/* Make a copy of the callback info. */

		if ((copy = _DtCmsMakeRegistrationInfo(user->name, 0,
		    r->prognum, r->versnum, r->procnum, r->pid)) == NULL) {
			*res = failed_4;
			_DtCmsUnlockCalendar(cal);
			return (1);
		}

        	/* Store it away so that it can later be called. */
		copy->next = cal->rlist;
		cal->rlist = copy;

		if (debug) {
			fprintf(stderr, "%s requested registration on %s. registered pid= %d\n",
				user->name, r->target, r->pid);
			_DtCmsListRegistration(cal->rlist,
					cal->calendar);
		}
		*res = registered_4;
		_DtCmsUnlockCalendar(cal);
        	return(1);
	} else {
		/* already registered */
		*res = registered_4;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}
}

/* de-register an rpc callback proc from the client */
extern bool_t
_DtCm_deregister_callback_4_svc(Registration_4 *r, Registration_Status_4 *res, struct svc_req *svcrq)
{
	CSA_return_code stat;
	_DtCmsCalendar *cal;
	_DtCmsSender *user;
        _DtCmsRegistrationInfo *p = NULL, *q = NULL;

	if (debug)
		fprintf(stderr, "_DtCm_deregister_callback_4_svc called\n");

        if (r->target == NULL) {
		*res = failed_4;
                return(1);
	}
 
	if ((stat = _DtCmsGetClientInfo(svcrq, &user)) != CSA_SUCCESS) {
		*res = csastat2regstat(stat);
		return (1);
	}

	if ((stat = _DtCmsFindCalendarInList(r->target, &cal))
	    != CSA_SUCCESS) {
		*res = csastat2regstat(stat);
		return (1);
	}

	if (cal == NULL) {
		*res = failed_4;
		return (1);
	}

	q = p = cal->rlist;
	while (p != NULL) {

		/* This says:
		 * 1) if the name of the caller requesting deregistration
		 * is the same as the original caller who requested
		 * requested registration, and
		 * 2) if the (transient) program, version, & procnum match
		 * the original registration, and
		 * 3) if the process id of the client matches the
		 *  orignal registration 
		 *  
		 *  ... only then is it ok to decommission the ticket.
		 */


		if ((strcmp(p->client, user->name)==0) &&
		    (p->prognum==r->prognum) &&
		    (p->versnum==r->versnum) &&
		    (p->procnum==r->procnum) &&
		    (p->pid==r->pid)) {	/* a match */
			if (debug) {
				fprintf(stderr, "%s requested deregistration on %s. registered pid= %d\n", user->name, r->target, r->pid);
			}
			if (p==q)
				cal->rlist = p->next;
			else
				q->next = p->next;
			_DtCmsFreeRegistrationInfo(p);
			if (debug) {
				_DtCmsListRegistration(cal->rlist,
					cal->calendar);
			}
			*res = deregistered_4;
			_DtCmsUnlockCalendar(cal);
			return(1);
		}
		q = p;
		p = p->next;
	}

	/* not found */
	*res = failed_4;
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern bool_t
_DtCm_rtable_set_access_4_svc(Access_Args_4 *args, Access_Status_4 *res, struct svc_req *svcrq)
{
	CSA_return_code		stat;
	_DtCmsSender		*user;
	CSA_flags		access;
	_DtCmsCalendar		*cal;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_set_access_4_svc called\n");

	*res = access_other_4;

	if (args->target == NULL)
		return (1);

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		*res = csastat2accessstat(stat);
		return(1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !(access&(CSA_OWNER_RIGHTS|CSA_CHANGE_CALENDAR_ATTRIBUTES))) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !(access & CSA_OWNER_RIGHTS))) {

		*res = access_failed_4;
		_DtCmsUnlockCalendar(cal);
		return (1);
        }

	/* Verify that the callog file has not been modified by
	 * something other than rpc.cmsd.
	 */
	if ((stat = _DtCmsVerifyLogFileInfo(cal->calendar, cal->modtime, cal->blksize)) != CSA_SUCCESS) {
		*res = csastat2accessstat(stat);
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		cms_attribute		attr;
		cms_attribute_value	attrval;
		cms_access_entry	*alist;

		if (args->access_list && (alist =
		    _DtCmsConvertV4AccessList(args->access_list,
				cal->attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I]\
				.value->item.calendar_user_value)) == NULL) {
			*res = csastat2accessstat(CSA_E_INSUFFICIENT_MEMORY);
			_DtCmsUnlockCalendar(cal);
			return (1);
		}

		attr.name.name = CSA_CAL_ATTR_ACCESS_LIST;
		attr.name.num = CSA_CAL_ATTR_ACCESS_LIST_I;
		attr.value = &attrval;
		attrval.type = CSA_VALUE_ACCESS_LIST;
		attrval.item.access_list_value = alist;

		stat = _DtCmsUpdateCalAttributesAndLog(cal, 1, &attr, access);
	} else {
		stat = _DtCmsSetV4AccessListAndLog(cal, args->access_list);
	}

	if ((*res = csastat2accessstat(stat)) == access_ok_4) {

		cms_attribute	attr;

		/* Write has been successful.  Update the modtime and
		 * blksize info, since the callog file has been updated.
		 */
		_DtCmsUpdateLogFileInfo(cal->calendar, &(cal->modtime),
			&(cal->blksize));

		_DtCmsMarkCheckOwner(cal);

		attr.name.name = CSA_CAL_ATTR_ACCESS_LIST;
		cal->rlist = _DtCmsDoUpdateCalAttrsCallback(cal->rlist,
				cal->calendar, user->name, 1, &attr, -1);
	}

	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern bool_t
_DtCm_rtable_get_access_4_svc(Access_Args_4 *args, Access_Args_4 *res, struct svc_req *svcrq)
{
	CSA_return_code		stat;
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;
	CSA_flags		access;
	boolean_t		useronly = _B_FALSE;
	cms_access_entry	aentry;
	char *name, *ptr;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_get_access_4_svc called\n");

	res->target = NULL;
	res->access_list = (Access_Entry_4 *) NULL;

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		return (1);
	}

	/* v4 clients (mainly cm) needs the access right information for
	 * the user to do the right thing, so return the information for
	 * that particular user only.
	 */
	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_VIEW_CALENDAR_ATTR_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !(access & CSA_OWNER_RIGHTS))) {
		useronly = _B_TRUE;
		name = strdup(user->name);
		if ((ptr = strchr(name, '@')) != NULL)
			ptr[0] = '\0';
	}


	if (cal->fversion > 1) {
		if (useronly) {
			aentry.user = name;
			aentry.rights = access;
			aentry.next = NULL;
			res->access_list = _DtCmsConvertV5AccessList(&aentry,
						_B_FALSE);
			free(name);
		} else {
			res->access_list = _DtCmsConvertV5AccessList(
					cal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].\
					value->item.access_list_value, _B_FALSE);
		}
	} else {
		if (useronly) {
			res->access_list = _DtCm_make_access_entry4(name, access);
			free(name);
		} else {
			_DtCmsCopyAccessList4(cal->alist, &res->access_list);
		}

		if (debug)
			_DtCmsShowAccessList (res->access_list);
	}

	res->target = strdup(args->target); 

	_DtCmsUnlockCalendar(cal);
	return (1);
}

extern bool_t
_DtCm_rtable_abbreviated_lookup_key_range_4_svc(
	Table_Args_4	*args,
	Table_Res_4     *res,
	struct svc_req	*svcrq)
{
	CSA_return_code	stat;
	_DtCmsCalendar	*cal;
	_DtCmsSender	*user;
	CSA_flags	access;
	Keyrange_4	*p_range;
	Abb_Appt_4	*abbr_r = NULL;

	if (debug)
		fprintf(stderr,
		    "_DtCm_rtable_abbreviated_lookup_key_range_4_svc called\n");

	res->res.tag = AB_4;
	res->res.Table_Res_List_4_u.b = NULL;

	res->status = access_other_4;
	if ((p_range = args->args.Args_4_u.keyrange) == NULL)
		return (1);

	if ((stat = _DtCmsLoadAndCheckAccess(svcrq, args->target, &user,
	    &access, &cal)) != CSA_SUCCESS) {
		res->status = csastat2accessstat(stat);
		return (1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_VIEW_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_BROWSE_ACCESS(access))) {
		res->status = access_failed_4;
		_DtCmsUnlockCalendar(cal);
		return (1);
	}

	while (p_range != NULL)
	{

		if ((stat = _DtCmsLookupKeyrangeV4(cal, user->name, access,
		    _B_FALSE, _B_TRUE, p_range->tick1, p_range->tick2, 0, 0,
		    p_range->key, NULL, 0, NULL, NULL, NULL, &abbr_r))
		    != CSA_SUCCESS) {
			break;
		}

		p_range = p_range->next;
	}

	if (stat == CSA_SUCCESS)
		res->res.Table_Res_List_4_u.b = abbr_r;
	else
		_DtCm_free_abbrev_appt4(abbr_r);

	res->status = csastat2accessstat(stat);
	_DtCmsUnlockCalendar(cal);
	return (1);
}

extern bool_t
_DtCm_rtable_gmtoff_4_svc(void *args, long *res, struct svc_req *svcrq)
{
	extern long timezone;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_gmtoff_4_svc called\n");

	*res = timezone;
	return(1);
}

extern bool_t
_DtCm_rtable_create_4_svc(Table_Op_Args_4 *args, Table_Status_4 *res, struct svc_req *svcrq)
{
	CSA_return_code stat;
	_DtCmsCalendar	*cal;
	_DtCmsSender	*source;
	char		*calname, *username, *log, *realuser;
	char		*ptr;
	Access_Entry_4 aentry;
	boolean_t	isuser;
	char		buf[BUFSIZ];

	if (debug)
		fprintf(stderr, "_DtCm_rtable_create_4_svc called\n");

	*res = other_4;
	if ((stat = _DtCmsGetClientInfo(svcrq, &source)) != CSA_SUCCESS) {
		*res = csastat2tablestat(stat);
		return (1);
	}

	/* check whether the calling user can create calendar */
	if ((stat = _DtCmsCheckCreateAccess(source)) != CSA_SUCCESS) {
		*res = csastat2tablestat(stat);
		return (1);
	}

	/*
	 * If the calendar name is a user name,
	 * make sure it's the same as sender.
	 * unless the sender is the local super user.
	 * also, remote root cannot create root@localhost
	 */
	calname = _DtCmGetPrefix(args->target, '@');
	username = _DtCmGetPrefix(source->name, '@');
	isuser = _DtCmIsUserName(calname);
	if (source->localroot == _B_FALSE &&
	    ((strcmp(calname, _DTCMS_SUPER_USER) == 0) ||
	    (isuser == _B_TRUE && strcmp(calname, username)))) {
		free(username);
		free(calname);
		*res = denied_4;
		return(1);
	}

	log = _DtCmsGetLogFN(calname);
	free(calname);
	free(username);

	if (source->localroot == _B_TRUE && isuser == _B_TRUE)
		realuser = args->target;
	else
		realuser = source->name;

	/* add host name if it's not given */
	if ((ptr = strchr(realuser, '@')) == 0) {
		ptr = strchr(source->name, '@');
		sprintf(buf, "%s@%s", realuser, ++ptr);
	} else
		strcpy(buf, realuser);

	/* add the calendar to the list if it doesn't exist already */
        if ((stat = _DtCmsAddCalendarToList(args->target, &cal))
	    != CSA_SUCCESS) {
	  *res = csastat2tablestat(stat); 
	  return(1);
	}

	if ((stat = _DtCmsCreateLogV1(buf, log)) != CSA_SUCCESS) {
		*res = csastat2tablestat(stat);
		_DtCmsFreeCalendar(cal, TRUE);
	}
	else {
		*res = ok_4;

		/* initialize the access list to be "world", access_read_4 */
		aentry.next = NULL;
		aentry.who = WORLD;
		aentry.access_type = access_read_4;
		if ((stat = _DtCmsAppendAccessByFN(log, &aentry))
		    != CSA_SUCCESS) {
			unlink(log);
			*res = csastat2tablestat(stat);
			_DtCmsFreeCalendar(cal, TRUE);
		}
	}

	/* Read in the changes we just made to the calendar file. */
	if ((stat = _DtCmsLoadCalendar(cal)) != CSA_SUCCESS) {
	  *res = csastat2tablestat(stat);
	  _DtCmsFreeCalendar(cal, TRUE);
	}

	free(log);
	_DtCmsUnlockCalendar(cal);
	return (1);
}

extern bool_t
_DtCm_rtable_remove_4_svc(Table_Op_Args_4 *args, Table_Status_4 *res, struct svc_req *svcrq)
{
	*res = tbl_notsupported_4;
	return (1);
}

extern bool_t
_DtCm_rtable_rename_4_svc(Table_Op_Args_4 *args, Table_Status_4 *res, struct svc_req *svcrq)
{
	*res = tbl_notsupported_4;
	return (1);
}

extern bool_t
_DtCm_rtable_ping_4_svc(void *args, void *res, struct svc_req *svcrq)
{
	char dummy;

	if (debug)
		fprintf(stderr, "_DtCm_rtable_ping_4_svc called\n");

	res = (void *)dummy;
	return(1); /* for RPC reply */
}

extern void
initrtable4(program_handle ph)
{
        ph->program_num = TABLEPROG;
        ph->prog[TABLEVERS_4].vers = &tableprog_4_table[0];
        ph->prog[TABLEVERS_4].nproc = sizeof(tableprog_4_table)/sizeof(tableprog_4_table[0]);
}

/******************************************************************************
 * static functions used within the file
 ******************************************************************************/

static Access_Status_4
csastat2accessstat(CSA_return_code stat)
{
	switch (stat) {
	case CSA_SUCCESS:
		return (access_ok_4);
	case CSA_E_CALENDAR_EXISTS:
		return (access_exists_4);
	case CSA_E_CALENDAR_NOT_EXIST:
		return (access_notable_4);
	case CSA_E_INSUFFICIENT_MEMORY:
	case CSA_E_NO_AUTHORITY:
		return (access_failed_4);
	case CSA_X_DT_E_BACKING_STORE_PROBLEM:
	case CSA_E_DISK_FULL:
		return (access_incomplete_4);
	case CSA_E_NOT_SUPPORTED:
		return (access_notsupported_4);
	case CSA_E_INVALID_PARAMETER:
	case CSA_E_FAILURE:
	case CSA_X_DT_E_ENTRY_NOT_FOUND:
	default:
		return (access_other_4);
	}
}

static Table_Status_4
csastat2tablestat(CSA_return_code stat)
{
	switch (stat) {
	case CSA_SUCCESS:
		return (ok_4);
	case CSA_E_CALENDAR_EXISTS:
		return (tbl_exist_4);
	case CSA_E_CALENDAR_NOT_EXIST:
		return (notable_4);
	case CSA_E_NOT_SUPPORTED:
		return (tbl_notsupported_4);
	case CSA_E_NO_AUTHORITY:
		return (denied_4);
	case CSA_E_FAILURE:
	case CSA_E_INSUFFICIENT_MEMORY:
	case CSA_E_INVALID_PARAMETER:
	default:
		return (other_4);
	}
}

static Registration_Status_4
csastat2regstat(CSA_return_code stat)
{
	switch (stat) {
	case CSA_E_CALENDAR_NOT_EXIST:
		return(reg_notable_4);
	default:
		return (failed_4);
	}
}

