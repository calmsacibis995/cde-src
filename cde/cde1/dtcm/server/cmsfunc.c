/* $XConsortium: cmsfunc.c /main/cde1_maint/5 1995/10/31 10:28:51 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)cmsfunc.c	1.48 96/06/21 Sun Microsystems, Inc."

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include <rpc/rpc.h>
#include <dirent.h>
#ifdef MT
#include <pthread.h>
#endif
#include "csa.h"
#include "cm.h"
#include "rtable4.h"
#include "attr.h"
#include "access.h"
#include "cmscalendar.h"
#include "updateattrs.h"
#include "cmsdata.h"
#include "cmsentry.h"
#include "match.h"
#include "rpcextras.h"
#include "v5ops.h"
#include "v4ops.h"
#include "iso8601.h"
#include "log.h"
#include "lutil.h"
#include "cm_tbl.i"
#include "callback.h"
#include "insert.h"
#include "update.h"
#include "delete.h"
#include "lookup.h"
#include "misc.h"
#include "convert4-5.h"
#include "convert5-4.h"
#include "appt4.h"
#include "cmsconvert.h"

extern int debug;

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/

static CSA_return_code _DtCmsCreateCallog(_DtCmsSender *user,
					cms_create_args *args,
					_DtCmsCalendar *cal);
static CSA_return_code _ListCalendarNames(CSA_uint32 *num_names,
					char ***names);
static void * _grow_char_array(void *ptr, CSA_uint32 oldcount,
					CSA_uint32 newcount);
static void _free_char_array(CSA_uint32 num_elem, char **ptr);
static CSA_return_code _DtCmsGetOldCalAttrNames(_DtCmsCalendar *cal,
					CSA_uint32 *num_names_r,
					cms_attr_name **names_r);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

extern bool_t
cms_ping_5_svc(void *args, void *res, struct svc_req *svcrq)
{
	if (debug)
		fprintf(stderr, "cms_ping_5_svc called\n");

	res = NULL;

	return(1);
}


extern bool_t
cms_list_calendars_5_svc(void *args, cms_list_calendars_res *res, struct svc_req *svcrq)
{
        if (debug)
		fprintf(stderr, "cms_list_calendars_5_svc called\n");
	
	res->num_names = 0;

	res->stat = _ListCalendarNames(&res->num_names, &res->names);

	return(1);
}

extern bool_t
cms_open_calendar_5_svc(cms_open_args *args, cms_open_res *res, struct svc_req *svcrq)
{
        _DtCmsCalendar		*cal;
	_DtCmsSender		*user;

	if (debug)
		fprintf(stderr, "cms_open_calendar_5_svc called\n");
 
	res->num_attrs = 0;

	/* check parameter */
	if (args->cal == NULL) {
		res->stat = CSA_E_INVALID_PARAMETER;
		return(1);
	}

	if ((res->stat = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user, &res->user_access, &cal)) != CSA_SUCCESS)
	  return(1);


	if (res->user_access == NULL) {
		res->stat = CSA_E_NO_AUTHORITY;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	res->svr_vers = TABLEVERS;

	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {

		res->file_vers = cal->fversion;

	} else {
		/* old format data */
		res->file_vers = _DtCM_FIRST_EXTENSIBLE_DATA_VERSION - 1;
	}

	/* return attribute names */
	if ((res->stat = _DtCmsGetAllCalAttrs(cal, &res->num_attrs, &res->attrs,
	    _B_FALSE)) == CSA_SUCCESS) {

		/* do callbacks */
		cal->rlist = _DtCmsDoOpenCalCallback(cal->rlist,
				cal->calendar, user->name, args->pid);
	}
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_create_calendar_5_svc(cms_create_args *args, CSA_return_code *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;

	if (debug)
		fprintf(stderr, "cms_create_calendar_5_svc called\n");

	/* check parameter */
	if (args->cal == NULL) {
		*res = CSA_E_INVALID_PARAMETER;
		return(1);
	}

	/* need to check whether we know about the sender,
	 * if not, fail the request
	 */
	if ((*res = _DtCmsGetClientInfo(svcrq, &user)) != CSA_SUCCESS) {
		return(1);
	}

	/* check whether the calling user can create calendar */
	if ((*res = _DtCmsCheckCreateAccess(user)) != CSA_SUCCESS)
		return(1);

	if ((*res = _DtCm_check_cal_cms_attributes(_DtCMS_VERSION4,
	    args->num_attrs, args->attrs, user->name, args->cal, _B_TRUE, _B_TRUE, _B_FALSE)) != CSA_SUCCESS)
		return(1);

	if ((*res = _DtCmsAddCalendarToList(args->cal, &cal))
	    != CSA_SUCCESS) {

		return(1);

	} else {

		/* create callog file for new calendar */
		if ((*res = _DtCmsCreateCallog(user, args, cal)) == CSA_SUCCESS) {
			char         *filename;
			struct stat   statbuf;

			/* Get the modification time and block size of the
			 * calendar. This information will be used later when
			 * writing out appointments back to the callog file.
			 */
			filename = _DtCmsGetLogFN(cal->calendar);
			if (stat(filename, &statbuf) == -1) {
		
				/* This was already done in the fopen() call
				 * previous to the stat() call; however, this
				 * check is here just in case something happens
				 * in between the fopen() and the stat() call.
				 */
				if (errno == ENOENT)
				      *res = CSA_E_CALENDAR_NOT_EXIST;
				else
				      *res = CSA_X_DT_E_BACKING_STORE_PROBLEM;
				_DtCmsUnlockCalendar(cal);
				return(1);
			}
			free(filename);
			cal->modtime = statbuf.st_mtime;
			cal->blksize = statbuf.st_blksize;
#ifdef UNDEF
			if (debug) {
		  		char cbuf[26];

		  		ctime_r(&(cal->modtime), cbuf);
				fprintf(stderr,
					"Log file created:\n\tmodtime: %s\n\tblksize: %d\n",
					cbuf, cal->blksize);
			}
#endif
			_DtCmsUnlockCalendar(cal);
		}
	}
	return(1);
}

extern  bool_t
cms_remove_calendar_5_svc(cms_remove_args *args, CSA_return_code *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;
	CSA_flags		access;

	if (debug)
		fprintf(stderr, "cms_remove_calendar_5_svc called\n");

	if ((*res = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
					       &access, &cal)) != CSA_SUCCESS)
	  return(1);

	if (!(access & CSA_OWNER_RIGHTS)) {

	        _DtCmsUnlockCalendar(cal);
	        *res = CSA_E_NO_AUTHORITY;

	} else {

		/* move callog.name file to calrm.name */
		*res = _DtCmsRemoveLog(cal->calendar, user->name);

		if (*res != CSA_SUCCESS) {
		  _DtCmsUnlockCalendar(cal);
		  return(1);
		}

		/* do callbacks */
		cal->rlist = _DtCmsDoRemoveCalCallback(cal->rlist,
				cal->calendar, user->name, args->pid);

		/* free up internal structures */
		_DtCmsFreeCalendar(cal, FALSE);
	} 

	return(1);
}

extern  bool_t
cms_register_5_svc(cms_register_args *args, CSA_return_code *res, struct svc_req *svcrq)
{
	_DtCmsSender		*user;
	_DtCmsCalendar		*cal;
	_DtCmsRegistrationInfo	*rinfo;

	if (debug)
		fprintf(stderr, "cms_register_5_svc called\n");

	if (args->cal == NULL ||
	    args->update_type >= (CSA_CB_ENTRY_UPDATED << 1)) {
		*res = CSA_E_INVALID_PARAMETER;
		return(1);
	}

	if ((*res = _DtCmsGetClientInfo(svcrq, &user)) != CSA_SUCCESS)
		return(1);

	if ((*res = _DtCmsGetCalendarByName(args->cal, &cal))
	    != CSA_SUCCESS)
	  return(1);

	if ((rinfo = _DtCmsGetRegistration(&(cal->rlist), user->name, args->prognum,
	    args->versnum, args->procnum, args->pid)) == NULL) {
		/* this client has not registered */

		if ((rinfo = _DtCmsMakeRegistrationInfo(user->name,
		    args->update_type, args->prognum, args->versnum,
		    args->procnum, args->pid)) == NULL) {
			*res = CSA_E_INSUFFICIENT_MEMORY;
			_DtCmsUnlockCalendar(cal);
			return(1);
		}

		/* put in the calendar's registration list */
		rinfo->next = cal->rlist;
		cal->rlist = rinfo;
	} else {
		if (debug) {
			fprintf(stderr, "%s registered on %s, old types = %d\n",
				user->name, args->cal, rinfo->types);
		}

		/* add new type to the registration */
		rinfo->types = rinfo->types | args->update_type;
	}

	if (debug) {
		fprintf(stderr, "%s registered on %s, types = %d\n",
			user->name, args->cal, rinfo->types);
	}
	_DtCmsUnlockCalendar(cal);
	*res = CSA_SUCCESS;
	return(1);
}

extern  bool_t
cms_unregister_5_svc(cms_register_args *args, CSA_return_code *res, struct svc_req *svcrq)
{
	_DtCmsSender		*user;
	_DtCmsCalendar		*cal;
	_DtCmsRegistrationInfo	*rinfo;

	if (debug)
		fprintf(stderr, "cms_unregister_5_svc called\n");

	if (args->cal == NULL ||
	    args->update_type >= (CSA_CB_ENTRY_UPDATED << 1)) {
		*res = CSA_E_INVALID_PARAMETER;
		return(1);
	}

	if ((*res = _DtCmsGetClientInfo(svcrq, &user)) != CSA_SUCCESS)
		return(1);

	if ((*res = _DtCmsFindCalendarInList(args->cal, &cal))
	    != CSA_SUCCESS)
	  return(1);

	if (cal == NULL) {
	  *res = CSA_E_CALLBACK_NOT_REGISTERED;
	  return(1);
	}
	  
	if ((rinfo = _DtCmsGetRegistration(&(cal->rlist), user->name,
	    args->prognum, args->versnum, args->procnum, args->pid)) == NULL) {
		*res = CSA_E_CALLBACK_NOT_REGISTERED;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* update registration info */
	if (debug) {
		fprintf(stderr, "%s registered on %s, old types = %d\n",
			user->name, args->cal, rinfo->types);
	}

	/* registered bits are cleared, unregistered bits are ignored */
	rinfo->types = (rinfo->types | args->update_type) ^ args->update_type;

	if (debug) {
		fprintf(stderr, "%s unregistered types %d on %s, new types = %d\n",
			user->name, args->update_type, args->cal, rinfo->types);
	}

	if (rinfo->types == 0) {
		cal->rlist = _DtCmsRemoveRegistration(cal->rlist, rinfo);
	}
	_DtCmsUnlockCalendar(cal);
	*res = CSA_SUCCESS;
	return(1);
}

extern  bool_t
cms_enumerate_calendar_attr_5_svc(buffer *args, cms_enumerate_calendar_attr_res *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;
	CSA_flags		access;


	if (debug)
		fprintf(stderr, "cms_enumerate_calendar_attr_5_svc called\n");

	res->num_names = 0;

	if ((res->stat = _DtCmsLoadAndCheckAccess(svcrq, *args, &user,
						    &access, &cal)) != CSA_SUCCESS) {
	  return(1);
	}

	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {

		if (_DTCMS_HAS_VIEW_CALENDAR_ATTR_ACCESS(access)) {
			res->stat = _DtCmsGetCalAttrNames(cal, &res->num_names,
					&res->names);
		} else {
			res->stat = CSA_E_NO_AUTHORITY;
		}

	} else if (access & CSA_OWNER_RIGHTS) { 
		/* this is the old format calendar, only owner can do this */
		res->stat = _DtCmsGetOldCalAttrNames(cal, &res->num_names,
				&res->names);
	} else {
		res->stat = CSA_E_NO_AUTHORITY;
	}
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_get_calendar_attr_5_svc(cms_get_cal_attr_args *args, cms_get_cal_attr_res *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;
	CSA_flags			access;

	if (debug)
		fprintf(stderr, "cms_get_calendar_attr_5_svc called\n");

	res->num_attrs = 0;

	if ((res->stat = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
						    &access, &cal)) != CSA_SUCCESS) {
	  return(1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_VIEW_CALENDAR_ATTR_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !(access & CSA_OWNER_RIGHTS))) {
		res->stat = CSA_E_NO_AUTHORITY;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	if (args->num_names > 0)
		res->stat = _DtCmsGetCalAttrsByName(cal, args->num_names,
				args->names, &res->num_attrs, &res->attrs);
	else
		res->stat = _DtCmsGetAllCalAttrs(cal, &res->num_attrs,
				&res->attrs, _B_TRUE);
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_set_calendar_attr_5_svc(cms_set_cal_attr_args *args, CSA_return_code *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;
	CSA_flags		access;

	if (debug)
		fprintf(stderr, "cms_set_calendar_attr_5_svc called\n");

	if ((*res = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
					       &access, &cal)) != CSA_SUCCESS) {
	  return(1);
	}

	if ((cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !(access & CSA_OWNER_RIGHTS)) ||
	    (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !(access & (CSA_OWNER_RIGHTS | CSA_INSERT_CALENDAR_ATTRIBUTES |
	    CSA_CHANGE_CALENDAR_ATTRIBUTES)))) {
		*res = CSA_E_NO_AUTHORITY;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* check argument */
	if (args->cal == NULL || args->num_attrs == 0) {
		*res = CSA_E_INVALID_PARAMETER;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* check validity of attribute values */
	if ((*res = _DtCm_check_cal_cms_attributes(
	    (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION ?
	    cal->fversion : _DtCM_FIRST_EXTENSIBLE_DATA_VERSION - 1),
	    args->num_attrs, args->attrs, NULL, NULL, _B_TRUE, _B_FALSE, _B_TRUE))
	    != CSA_SUCCESS) {
	  _DtCmsUnlockCalendar(cal);
	  return(1);
	}

	/* Verify that the callog file has not been modified by
	 * something other than rpc.cmsd.
	 */
	if ((*res = _DtCmsVerifyLogFileInfo(cal->calendar, cal->modtime, cal->blksize)) != CSA_SUCCESS) {
	        _DtCmsUnlockCalendar(cal);
		return(1);
	}

	if (cal->fversion >=_DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		*res = _DtCmsUpdateCalAttributesAndLog(cal, args->num_attrs,
		    	args->attrs, access);
	} else {
		int		i;
		Access_Entry_4	*v4list;

		/* for old format file, the only settable calendar attribute
		 * is access list
		 */
		for (i = args->num_attrs - 1; i >= 0; i--) {
			if (args->attrs[i].name.name)
				break;
		}

		if (args->attrs[i].value == NULL ||
		    args->attrs[i].value->item.access_list_value == NULL) {
			*res = _DtCmsSetV4AccessListAndLog(cal, NULL);
		} else if ((*res = _DtCmsCmsAccessToV4Access(
		    args->attrs[i].value->item.access_list_value,
		    &v4list)) == CSA_SUCCESS) {
			*res = _DtCmsSetV4AccessListAndLog(cal, v4list);
		}
	}

	if (*res == CSA_SUCCESS) {
		_DtCmsMarkCheckOwner(cal);

		/* Write has been successful.  Update the modtime and
		 * blksize info, since the callog file has been updated.
		 */
		_DtCmsUpdateLogFileInfo(cal->calendar, &(cal->modtime),
			&(cal->blksize));

		/* do callback */
		cal->rlist = _DtCmsDoUpdateCalAttrsCallback(cal->rlist,
				cal->calendar, user->name, args->num_attrs,
				args->attrs, args->pid);
	}
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_archive_5_svc(cms_archive_args *args, cms_archive_res *res, struct svc_req *svcrq)
{
	if (debug)
		fprintf(stderr, "cms_archive_5_svc called\n");

	res->stat = CSA_E_NOT_SUPPORTED;
	return(1);
}

extern  bool_t
cms_restore_5_svc(cms_restore_args *args, CSA_return_code *res, struct svc_req *svcrq)
{
	if (debug)
		fprintf(stderr, "cms_restore_5_svc called\n");

	*res = CSA_E_NOT_SUPPORTED;
	return(1);
}

extern  bool_t
cms_lookup_reminder_5_svc(cms_reminder_args *args, cms_reminder_res *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;
	CSA_flags		access;

	if (debug)
		fprintf(stderr, "cms_lookup_reminder_5_svc called\n");

	res->rems = NULL;

	if ((res->stat = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
						  &access, &cal)) != CSA_SUCCESS) {
	  return(1);
	}

	if ((access & CSA_OWNER_RIGHTS) == 0) {
		res->stat = CSA_E_NO_AUTHORITY;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		res->stat = _DtCmsLookupReminder(cal->remq, args->tick,
				args->num_names, args->names, &res->rems);
	} else {
		Reminder_4	*v4rem;
		_DtCmsEntryId	*ids;

		if ((res->stat = _DtCmsGetV4Reminders(cal, args->tick, &v4rem,
		    &ids)) == CSA_SUCCESS) {
			res->stat = _DtCmsV4ReminderToReminderRef(cal->calendar,
					v4rem, ids, &res->rems);
			_DtCm_free_reminder4(v4rem);
			_DtCmsFreeEntryIds(ids);
		}
	}
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_lookup_entries_5_svc(cms_lookup_entries_args *args, cms_entries_res *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;
	CSA_flags		access;
	time_t			start1, start2, end1, end2;
	long                    id = 0;
	boolean_t		no_match;
	boolean_t		no_start_time_range, no_end_time_range;
	CSA_uint32		hnum;
	cms_attribute		*hattrs;
	CSA_enum		*hops;

	if (debug)
		fprintf(stderr, "cms_lookup_entries_5_svc called\n");
	
	res->entries = NULL;

	if ((res->stat = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
						    &access, &cal)) != CSA_SUCCESS) {
	  return(1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_VIEW_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_BROWSE_ACCESS(access))) {
		res->stat = CSA_E_NO_AUTHORITY;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* check operator */
	if ((res->stat = _DtCm_check_operator(args->num_attrs, NULL,
					      args->attrs, args->ops)) != CSA_SUCCESS) {
	  _DtCmsUnlockCalendar(cal);
	  return(1);
	}

	/* get time range */
	if ((res->stat = _DtCmHashCriteria(
	    cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION ?
	    _DtCm_entry_name_tbl : cal->entry_tbl, args->num_attrs,
	    NULL, args->attrs, args->ops, &no_match, &no_start_time_range,
	    &no_end_time_range, &start1, &start2, &end1, &end2, &id,
	    &hnum, &hattrs, &hops)) == CSA_E_INVALID_ATTRIBUTE) {

		/* attribute not defined in this calendar specified,
		 * return NULL list
		 */
		res->stat = CSA_SUCCESS;
		_DtCmsUnlockCalendar(cal);
		return(1);

	} else if (res->stat != CSA_SUCCESS || no_match == _B_TRUE) {
	  _DtCmsUnlockCalendar(cal);
	  return(1);
	}

	/* do lookup */
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		if (id > 0)
			res->stat = _DtCmsLookupEntriesById(cal, user->name, access,
					no_start_time_range, no_end_time_range,
					start1, start2, end1, end2, id, hnum,
					hattrs, hops, &res->entries);
		else
			res->stat = _DtCmsLookupEntries(cal, user->name, access,
					start1, start2, no_end_time_range,
					end1, end2, hnum, hattrs, hops,
					&res->entries);
	} else {
		Range_4	prange;
		Appt_4	*appt = NULL;;

		if (id > 0) {
			res->stat = _DtCmsLookupKeyrangeV4(cal, user->name, access,
					no_start_time_range, no_end_time_range,
					start1, start2, end1, end2, id,
					_DtCm_match_one_appt, hnum, hattrs,
					hops, &appt, NULL);
		} else {
			prange.key1 = start1;
			prange.key2 = start2;
			prange.next = NULL;
			res->stat = _DtCmsLookupRangeV4(cal, user->name, access,
					&prange, no_end_time_range, end1, end2,
					_DtCm_match_one_appt, hnum,
					hattrs, hops, &appt, NULL);
		}

		if (res->stat == CSA_SUCCESS && appt) {
			res->stat = _DtCmsAppt4ToCmsentriesForClient(args->cal,
					appt, &res->entries);
			_DtCm_free_appt4(appt);
		}
	}

	_DtCmFreeHashedArrays(hnum, hattrs, hops);
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_enumerate_sequence_5_svc(cms_enumerate_args *args, cms_entries_res *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;
	CSA_flags		access;

	if (debug)
		fprintf(stderr, "cms_enumerate_sequence_5_svc called\n");

	res->entries = NULL;

	if ((res->stat = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
						    &access, &cal)) != CSA_SUCCESS) {
	  return(1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_VIEW_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_BROWSE_ACCESS(access))) {
		res->stat = CSA_E_NO_AUTHORITY;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* do lookup */
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		res->stat = _DtCmsEnumerateSequenceById(cal, user->name, access,
				_B_FALSE, _B_TRUE, args->start, args->end,
				0, 0, args->id, 0, NULL, NULL, &res->entries);
	} else {
		Range_4 prange;
		Appt_4 *appt = NULL;
		res->stat = _DtCmsLookupKeyrangeV4(cal, user->name, access,
				_B_FALSE, _B_TRUE, args->start, args->end,
				0, 0, args->id, NULL, 0, NULL, NULL, &appt,
				NULL);

		if (res->stat == CSA_SUCCESS && appt) {
			res->stat = _DtCmsAppt4ToCmsentriesForClient(args->cal,
					appt, &res->entries);
			_DtCm_free_appt4(appt);
		}
	}
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_get_entry_attr_5_svc(cms_get_entry_attr_args *args, cms_get_entry_attr_res *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	_DtCmsSender		*user;
	CSA_flags			access;

	if (debug)
		fprintf(stderr, "cms_get_entry_attr_5_svc called\n");

	res->entries = NULL;

	if ((res->stat = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
						    &access, &cal)) != CSA_SUCCESS) {
	  return(1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_VIEW_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_BROWSE_ACCESS(access))) {
		res->stat = CSA_E_NO_AUTHORITY;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* check operator */
	if (args->num_keys == 0) {
		res->stat = CSA_E_INVALID_PARAMETER;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		res->stat = _DtCmsLookupEntriesByKey(cal, user->name, access,
			args->num_keys, args->keys, args->num_names,
			args->names, &res->entries);
	} else {
		res->stat = CSA_E_NOT_SUPPORTED;
	}
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_insert_entry_5_svc(cms_insert_args *args, cms_entry_res *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	cms_entry		*entry;
	cms_key			key;
	_DtCmsSender		*user;
	CSA_flags		access, needaccess;
	Appt_4			*appt;

	if (debug)
		fprintf(stderr, "cms_insert_entry_5_svc called\n");

	res->entry = NULL;

	if ((res->stat = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
						    &access, &cal)) != CSA_SUCCESS) {
	  return(1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_INSERT_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_WRITE_ACCESS(access))) {
		res->stat = CSA_E_NO_AUTHORITY;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* check argument */
	if (args->cal == NULL || args->num_attrs == 0) {
		res->stat = CSA_E_INVALID_PARAMETER;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* check validity of attribute values */
	/* the attribute names will be hashed up to the defined ones */
	if ((res->stat = _DtCm_check_entry_cms_attributes(
	    (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION ?
	    cal->fversion : _DtCM_FIRST_EXTENSIBLE_DATA_VERSION - 1),
	    args->num_attrs, args->attrs, CSA_CB_ENTRY_ADDED, _B_TRUE))
	    != CSA_SUCCESS) {
	  _DtCmsUnlockCalendar(cal);
	  return(1);
	}

	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		if ((res->stat = _DtCmsMakeHashedEntry(cal, args->num_attrs,
						       args->attrs, &entry)) != CSA_SUCCESS) {
		  _DtCmsUnlockCalendar(cal);
		  return(1);
		}

		if ((res->stat = _DtCmsCheckInitialAttributes(entry))
		    != CSA_SUCCESS) {
			_DtCm_free_cms_entry(entry);
			_DtCmsUnlockCalendar(cal);
			return(1);
		}

		/* check access rights */
		needaccess = _DtCmsClassToInsertAccess(entry);
		if ((access & (CSA_OWNER_RIGHTS | needaccess)) == 0) {
			_DtCm_free_cms_entry(entry);
			res->stat = CSA_E_NO_AUTHORITY;
			_DtCmsUnlockCalendar(cal);
			return(1);
		}

		/* set organizer */
		if ((res->stat = _DtCm_set_string_attrval(user->name,
		    &entry->attrs[CSA_ENTRY_ATTR_ORGANIZER_I].value,
		    CSA_VALUE_CALENDAR_USER)) != CSA_SUCCESS) {
			_DtCm_free_cms_entry(entry);
			_DtCmsUnlockCalendar(cal);
			return(1);
		}

		/* Verify that the callog file has not been modified by
		 * something other than rpc.cmsd.
		 */
		if ((res->stat = _DtCmsVerifyLogFileInfo(cal->calendar, cal->modtime, cal->blksize)) != CSA_SUCCESS) {
			_DtCm_free_cms_entry(entry);
			_DtCmsUnlockCalendar(cal);
			return (1);
		}

		/* insert entry and log it */
		if ((res->stat = _DtCmsInsertEntryAndLog(cal, entry))
		    != CSA_SUCCESS) {
			_DtCm_free_cms_entry(entry);
			_DtCmsUnlockCalendar(cal);
			return(1);
		}

		key = entry->key;
	} else {
		if ((appt = _DtCm_make_appt4(_B_TRUE)) == NULL) {
			res->stat = CSA_E_INSUFFICIENT_MEMORY;
			_DtCmsUnlockCalendar(cal);
			return(1);
		}
 
		if ((res->stat = _DtCmsAttrsToAppt4(args->num_attrs,
		    args->attrs, appt, _B_TRUE)) != CSA_SUCCESS) {
			_DtCm_free_appt4(appt);
			_DtCmsUnlockCalendar(cal);
			return(1);
		}

		if (appt->author) free(appt->author);
		if ((appt->author = strdup(user->name)) == NULL) {
			_DtCm_free_appt4(appt);
			_DtCmsUnlockCalendar(cal);
			return(1);
		}

		/*
		 * calculate the correct start day,
		 */
		_DtCms_adjust_appt_startdate(appt);

		/* Verify that the callog file has not been modified by
		 * something other than rpc.cmsd.
		 */
		if ((res->stat = _DtCmsVerifyLogFileInfo(cal->calendar, cal->modtime, cal->blksize)) != CSA_SUCCESS) {
			_DtCm_free_appt4(appt);
			return (1);
		}

		if ((res->stat = _DtCmsInsertApptAndLog(cal, appt))
		    != CSA_SUCCESS) {
			_DtCm_free_appt4(appt);
			_DtCmsUnlockCalendar(cal);
			return(1);
		}

		key.id = appt->appt_id.key;
		key.time = appt->appt_id.tick;
	}

	if (res->stat == CSA_SUCCESS) {
		cal->modified = _B_TRUE;

		/* Write has been successful.  Update the modtime and
		 * blksize info, since the callog file has been updated.
		 */
		_DtCmsUpdateLogFileInfo(cal->calendar, &(cal->modtime),
			&(cal->blksize));
	} else {
	  _DtCmsUnlockCalendar(cal);
	  return(1);
	}

	/* do callback */
	cal->rlist = _DtCmsDoV1CbForV4Data(cal->rlist, user->name, args->pid,
			&key, NULL);

	cal->rlist = _DtCmsDoInsertEntryCallback(cal->rlist, cal->calendar,
			user->name, key.id, args->pid);

	/* reply */
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    (res->stat = _DtCmsAppt4ToCmsentriesForClient(cal->calendar, appt,
	    &entry)) == CSA_SUCCESS)) {

		res->stat = _DtCmsGetCmsEntryForClient(entry, &res->entry,
				_B_FALSE);

		_DtCm_free_cms_entry(entry);
	}
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_update_entry_5_svc(cms_update_args *args, cms_entry_res *res, struct svc_req *svcrq)
{

	_DtCmsCalendar		*cal;
	cms_entry		*newentry;
	caddr_t			event;
	_DtCmsSender		*user;
	CSA_flags		access;
	Appt_4			*appt = NULL;

	if (debug)
		fprintf(stderr, "cms_update_entry_5_svc called\n");

	res->entry = NULL;

	if ((res->stat = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
						  &access, &cal)) != CSA_SUCCESS) {
	  return(1);
	}

	if ((cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_CHANGE_ACCESS(access)) ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_V4_WRITE_ACCESS(access))) {
	  res->stat = CSA_E_NO_AUTHORITY;
	  _DtCmsUnlockCalendar(cal);
	  return(1);
	}

	/* check argument */
	if (args->cal == NULL || args->entry.id <= 0 || args->num_attrs == 0) {
		res->stat = CSA_E_INVALID_PARAMETER;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	if (args->scope < CSA_SCOPE_ALL || args->scope > CSA_SCOPE_FORWARD) {
		res->stat = CSA_E_INVALID_ENUM;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* check validity of attribute values */
	/* the attribute names will be hashed up to the defined ones */
	if ((res->stat = _DtCm_check_entry_cms_attributes(
	    (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION ?
	    cal->fversion : _DtCM_FIRST_EXTENSIBLE_DATA_VERSION - 1),
	    args->num_attrs, args->attrs, CSA_CB_ENTRY_UPDATED, _B_TRUE))
	    != CSA_SUCCESS) {
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* get event from one-time event tree */
	event = (caddr_t)rb_lookup(cal->tree, (caddr_t)&args->entry);


	/* Verify that the callog file has not been modified by
	 * something other than rpc.cmsd.
	 */
	if ((res->stat = _DtCmsVerifyLogFileInfo(cal->calendar, cal->modtime, cal->blksize)) != CSA_SUCCESS) {
		_DtCmsUnlockCalendar(cal);
		return (1);
	}


	/* update entry and log it */
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		if (event != NULL || args->scope == CSA_SCOPE_ALL) {
			res->stat = _DtCmsUpdateEntry(cal, user->name, access,
					&args->entry, args->num_attrs,
					args->attrs, NULL, &newentry);
		} else {
			res->stat = _DtCmsUpdateInstances(cal, user->name, access,
					&args->entry, args->scope,
					args->num_attrs, args->attrs, NULL,
					&newentry);
		}
	} else {
		Options_4	opt;

		if (event != NULL || args->scope == CSA_SCOPE_ALL)
			opt = do_all_4;
		else
			opt = (args->scope == CSA_SCOPE_ONE) ?
				do_one_4 : do_forward_4;

		if (event == NULL && (event = hc_lookup(cal->list,
		    (caddr_t)&args->entry)) == NULL) {

			res->stat = CSA_X_DT_E_ENTRY_NOT_FOUND;

		} else if ((appt = _DtCm_copy_one_appt4((Appt_4 *)event))
		    == NULL) {

			res->stat = CSA_E_INSUFFICIENT_MEMORY; 

		} else {
			/* get rid of exceptions */
			_DtCm_free_excpt4(appt->exception);
			appt->exception = NULL;

			if ((res->stat = _DtCmsAttrsToAppt4(args->num_attrs,
			    args->attrs, appt, _B_TRUE)) == CSA_SUCCESS) {

				if (opt == do_all_4)
					res->stat = _DtCmsChangeAll(cal, user->name,
						access, (Id_4 *)&args->entry,
						appt, NULL);
				else
					res->stat = _DtCmsChangeSome(cal, user->name,
						access, (Id_4 *)&args->entry,
						appt, opt, NULL);

			}
		}
	}

	if (res->stat == CSA_SUCCESS) {
		cal->modified = _B_TRUE;

		/* Update has been successful.  Update the modtime and
		 * blksize info, since the callog file has been updated.
		 */
		_DtCmsUpdateLogFileInfo(cal->calendar, &(cal->modtime),
			&(cal->blksize));
	} else {
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* do callback */
	cal->rlist = _DtCmsDoV1CbForV4Data(cal->rlist, user->name, args->pid,
			&args->entry,
			(appt ? (cms_key *)&appt->appt_id : &newentry->key));

	cal->rlist = _DtCmsDoUpdateEntryCallback(cal->rlist, cal->calendar,
			user->name, (appt ? appt->appt_id.key : newentry->key.id),
			args->entry.id, args->scope, args->entry.time,
			args->pid);

	/* reply */
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION ||
	    (cal->fversion < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    (res->stat = _DtCmsAppt4ToCmsentriesForClient(cal->calendar, appt,
	    &newentry)) == CSA_SUCCESS)) {

		res->stat = _DtCmsGetCmsEntryForClient(newentry, &res->entry,
				_B_FALSE);

		_DtCm_free_cms_entry(newentry);
	}
	_DtCmsUnlockCalendar(cal);
	return(1);
}

extern  bool_t
cms_delete_entry_5_svc(cms_delete_args *args, CSA_return_code *res, struct svc_req *svcrq)
{
	_DtCmsCalendar		*cal;
	caddr_t			event;
	_DtCmsSender		*user;
	CSA_flags		access;

	if (debug)
		fprintf(stderr, "cms_delete_entry_5_svc called\n");

	if ((*res = _DtCmsLoadAndCheckAccess(svcrq, args->cal, &user,
					       &access, &cal)) != CSA_SUCCESS) {
		return(1);
	}

	/* for v3 data, authority check is done in the routines doing the
	 * deletion since we may need to check the organizer
	 */
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
	    !_DTCMS_HAS_CHANGE_ACCESS(access)) {
		*res = CSA_E_NO_AUTHORITY;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	/* check argument */
	if (args->cal == NULL || args->entry.id <= 0) {
		*res = CSA_E_INVALID_PARAMETER;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}

	if (args->scope < CSA_SCOPE_ALL || args->scope > CSA_SCOPE_FORWARD) {
		*res = CSA_E_INVALID_ENUM;
		_DtCmsUnlockCalendar(cal);
		return(1);
	}


	/* get event from one-time event tree */
	event = (caddr_t)rb_lookup(cal->tree, (caddr_t)&args->entry);

	/* Verify that the callog file has not been modified by
	 * something other than rpc.cmsd.
	 */
	if ((*res = _DtCmsVerifyLogFileInfo(cal->calendar, cal->modtime, cal->blksize)) != CSA_SUCCESS) {
		_DtCmsUnlockCalendar(cal);
		return (1);
	}

	/* delete entry and log it */
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		if (event != NULL || args->scope == CSA_SCOPE_ALL) {
			*res = _DtCmsDeleteEntryAndLog(cal, user->name, access,
				&args->entry, NULL);
		} else {
			*res = _DtCmsDeleteInstancesAndLog(cal, user->name, access,
				&args->entry, args->scope, NULL, NULL);
		}
	} else {
		if (event != NULL || args->scope == CSA_SCOPE_ALL) {
			*res = _DtCmsDeleteApptAndLog(cal, user->name, access,
				(Id_4 *)&args->entry, NULL);
		} else {
			*res = _DtCmsDeleteApptInstancesAndLog(cal, user->name, access,
				(Id_4 *)&args->entry,
				((args->scope == CSA_SCOPE_ONE) ? do_one_4 :
				do_forward_4), NULL, NULL);
		}
	}

	if (*res == CSA_SUCCESS) {
		cal->modified = _B_TRUE;

		/* Delete has been successful.  Update the modtime and
		 * blksize info, since the callog file has been updated.
		 */
		_DtCmsUpdateLogFileInfo(cal->calendar, &(cal->modtime),
			&(cal->blksize));
	} else {
	  _DtCmsUnlockCalendar(cal);
	  return(1);
	}

	/* do callback */
	cal->rlist = _DtCmsDoV1CbForV4Data(cal->rlist, user->name, args->pid,
			&args->entry, NULL);

	cal->rlist = _DtCmsDoDeleteEntryCallback(cal->rlist, cal->calendar,
			user->name, args->entry.id, args->scope,
			args->entry.time, args->pid);
	_DtCmsUnlockCalendar(cal);
	return(1);
}

void initfunctable(program_handle ph)
{
        ph->program_num = TABLEPROG;
        ph->prog[TABLEVERS].vers = &tableprog_5_table[0];
        ph->prog[TABLEVERS].nproc = sizeof(tableprog_5_table)/sizeof(tableprog_5_table[0]);
}

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

static CSA_return_code
_DtCmsCreateCallog(
	_DtCmsSender	*user,
	cms_create_args	*args,
	_DtCmsCalendar	*newcal)
{
	CSA_return_code	stat;
	char		name[BUFSIZ], datestr[80];
	char		*calname, *username, *log;
	cms_attribute_value val;
	cms_access_entry aentry;
	char		*realuser, *ptr;
	int		i;
	boolean_t	isuser;

	/*
	 * if calendar name is a user name, make sure that
	 * it's the same as the sender unless the sender is
	 * the local super user.
	 * Also, remote root cannot create root@localhost
	 */
	calname = _DtCmGetPrefix(args->cal, '@');
	username = _DtCmGetPrefix(user->name, '@');
	isuser = _DtCmIsUserName(calname);
	if (user->localroot == _B_FALSE &&
	    ((strcmp(calname, _DTCMS_SUPER_USER) == 0) ||
	    (isuser == _B_TRUE && strcmp(calname, username)))) {
		free(calname);
		free(username);
	        _DtCmsFreeCalendar(newcal, TRUE);
		return (CSA_E_NO_AUTHORITY);
	}
	log = _DtCmsGetLogFN(calname);
	free(calname);
	free(username);

	/* if owner is specified, and since it was checked to be correct,
	 * use it
	 */
	for (i = 0; i < args->num_attrs; i++) {
		if (args->attrs[i].name.num == CSA_CAL_ATTR_CALENDAR_OWNER_I)
			break;
	}

	if (args->num_attrs > 0 && i < args->num_attrs)
		realuser = args->attrs[i].value->item.string_value;
	else if (user->localroot == _B_TRUE && isuser == _B_TRUE)
		realuser = args->cal;
	else
		realuser = user->name;

	/* add host name if it's not given */
	if ((ptr = strchr(realuser, '@')) == 0) {
		ptr = strchr(user->name, '@');
		sprintf(name, "%s@%s", realuser, ++ptr);
	} else
		strcpy(name, realuser);

	/* create internal calendar data structure */
	if ((stat = _DtCmsInitCalendar(newcal, name)) != CSA_SUCCESS) {
		free(log);
		return(stat);
	}

	/* fill in information */
	_csa_tick_to_iso8601(time(0), datestr);

	if ((stat = _DtCm_set_string_attrval(datestr,
	    &newcal->attrs[CSA_CAL_ATTR_DATE_CREATED_I].value,
	    CSA_VALUE_DATE_TIME)) != CSA_SUCCESS) {
		_DtCmsFreeCalendar(newcal, TRUE);
		free(log);
		return (stat);
	}

	/* initialize access list to be "WORLD", VIEW_PUBLIC */
	aentry.user = WORLD;
	aentry.rights = CSA_VIEW_PUBLIC_ENTRIES;
	aentry.next = NULL;
	val.item.access_list_value = &aentry;
	val.type = CSA_VALUE_ACCESS_LIST;

	if ((stat = _DtCmUpdateAccessListAttrVal(&val,
	    &newcal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value)) != CSA_SUCCESS) {
		_DtCmsFreeCalendar(newcal, TRUE);
		free(log);
		return (stat);
	}

	/* set product identifier */
	if ((stat = _DtCm_set_string_attrval(_DtCM_PRODUCT_IDENTIFIER,
	    &newcal->attrs[CSA_CAL_ATTR_PRODUCT_IDENTIFIER_I].value,
	    CSA_VALUE_STRING)) != CSA_SUCCESS) {
		_DtCmsFreeCalendar(newcal, TRUE);
		free(log);
		return (stat);
	}

	/* set CSA version */
	if ((stat = _DtCm_set_string_attrval(_DtCM_SPEC_VERSION_SUPPORTED,
	    &newcal->attrs[CSA_CAL_ATTR_VERSION_I].value, CSA_VALUE_STRING))
	    != CSA_SUCCESS) {
		_DtCmsFreeCalendar(newcal, TRUE);
		free(log);
		return (stat);
	}

	/* initialize calendar attribute with info provided by caller */
	if ((stat = _DtCmsUpdateAttributes(args->num_attrs, args->attrs,
	    &newcal->num_attrs, &newcal->attrs, &newcal->cal_tbl, _B_TRUE, NULL))
	    != CSA_SUCCESS) {
		_DtCmsFreeCalendar(newcal, TRUE);
		free(log);
		return (stat);
	}

	/* use passed in char set if client does not supply one */
	if (&newcal->attrs[CSA_CAL_ATTR_CHARACTER_SET_I].value == NULL &&
	     args->char_set && *args->char_set != NULL) {
		if ((stat = _DtCm_set_string_attrval(args->char_set,
		    &newcal->attrs[CSA_CAL_ATTR_CHARACTER_SET_I].value,
		    CSA_VALUE_STRING)) != CSA_SUCCESS) {
			_DtCmsFreeCalendar(newcal, TRUE);
			free(log);
			return (stat);
		}
	}

	/* create file */
	if ((stat = _DtCmsCreateLogV2(name, log)) != CSA_SUCCESS) {
		_DtCmsFreeCalendar(newcal, TRUE);
		free(log);
		return (stat);
	}

	/* dump file */
	if ((stat = _DtCmsAppendCalAttrsByFN(log, newcal->num_attrs, newcal->attrs))
	    != CSA_SUCCESS) {
		free(log);
		unlink(log);
		_DtCmsFreeCalendar(newcal, TRUE);
		return (stat);
	}
	free(log);

	return (stat);
}

#define	_NAME_INCREMENT	10

static CSA_return_code
_ListCalendarNames(CSA_uint32 *num_names, char ***names)
{
	CSA_uint32	num = 0, count = 0;
	char	**names_r = NULL;
	DIR	*dirp;
	struct	dirent *dp;

#ifdef SunOS
	int	status;
#endif

	if ((dirp = opendir(".")) == NULL)
		return (CSA_E_FAILURE);

#ifdef SunOS
	if ((dp = (struct dirent *)malloc(sizeof(struct dirent)
					  + FILENAME_MAX)) == NULL) {
		closedir(dirp);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	while ((status = readdir_r(dirp, dp, &dp)) == 0) {
#else
	while (dp = readdir(dirp)) {
#endif
		if (strncmp(dp->d_name, "callog.", strlen("callog.")) == 0) {
			if (count == num) {
				count += _NAME_INCREMENT;
				if ((names_r = (char **)_grow_char_array(
				    names_r, sizeof(char *) * count,
				    sizeof(char *) * (count + _NAME_INCREMENT)))
				    == NULL) {
					if (num > 0) {
						_free_char_array(
							num, names_r);
					}
					closedir(dirp);
					return (CSA_E_INSUFFICIENT_MEMORY);
				}
			}
			if ((names_r[num++] = strdup(dp->d_name)) == NULL) {
				_free_char_array(num, names_r);
				closedir(dirp);
				return (CSA_E_INSUFFICIENT_MEMORY);
			}
		}
	}

	if (num > 0) {
		*num_names = num;
		*names = names_r;
	} else if (count > 0)
		free(names_r);

	return (CSA_SUCCESS);
}

static void *
_grow_char_array(void *ptr, CSA_uint32 oldcount, CSA_uint32 newcount)
{
	void *nptr;

	if (nptr = realloc(ptr, newcount)) {
		memset((void *)((char *)nptr + oldcount), NULL,
			newcount - oldcount);
		return (nptr);
	} else
		return (NULL);
}

static void
_free_char_array(CSA_uint32 num_elem, char **ptr)
{
	int i;

	if (num_elem == 0)
		return;

	for (i = 0; i < num_elem; i++) {
		if (ptr[i])
			free(ptr[i]);
		else
			break;
	}

	free(ptr);
}

/*
 * This routine assumes that the attributes are hashed already
 */
static CSA_return_code
_DtCmsGetOldCalAttrNames(
	_DtCmsCalendar	*cal,
	CSA_uint32	*num_names_r,
	cms_attr_name	**names_r)
{
	cms_attr_name	*names;
	CSA_uint32	i, j;

	if ((names = (cms_attr_name *)calloc(1,
	    sizeof(cms_attr_name)*_DtCM_OLD_CAL_ATTR_SIZE)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 1, j = 0; i <= _DtCM_DEFINED_CAL_ATTR_SIZE; i++) {
		if ((_CSA_cal_attr_info[i].fst_vers > 0 &&
		    _CSA_cal_attr_info[i].fst_vers <= cal->fversion)
		    || i == CSA_CAL_ATTR_CALENDAR_SIZE_I) {
			if ((names[j].name =
			    strdup(CSA_CALENDAR_ATTRIBUTE_NAMES[i])) == NULL)
			{
				_DtCmsFreeCmsAttrNames(j, names);
				return (CSA_E_INSUFFICIENT_MEMORY);
			} else {
				names[j].num = i;
				j++;
			}
		}
	}

	*num_names_r = j;
	*names_r = names;

	return (CSA_SUCCESS);
}

