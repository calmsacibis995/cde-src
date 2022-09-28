/* $XConsortium: calendar.c /main/cde1_maint/4 1995/10/31 09:50:06 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)calendar.c	1.24 97/01/31 Sun Microsystems, Inc."

/*****************************************************************************
 * Functions that manage the calendar data structures.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include "calendar.h"
#include "entry.h"
#include "cmsdata.h"
#include "agent_p.h"
#include "rpccalls.h"
#include "debug.h"
#include "attr.h"
#include "xtclient.h"
#include "misc.h"
#include "free.h"
#include "nametbl.h"
#include "lutil.h"

/*
 * A bug in a version of sun lib C, the weak version
 * of pthread_self() missing
 */
#ifdef SunOS
#include <thread.h>
#define pthread_self() thr_self()
#endif

/* linked list of calendar structure representing opened calendars */
/* the access field in _DtCm_active_cal_list contains the number of
 * calendar entries in the list
 */
Calendar _DtCm_active_cal_list;

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
static CSA_return_code _DtCm_lock_session(Calendar *cal, Calendar *prev,
					boolean_t not_again);

static CSA_return_code _get_owner_from_old_cal(Calendar *cal, char *buf);
static CSA_return_code _get_calendar_owner(Calendar *cal, uint num_names,
					char **names, CSA_attribute *attr);
static CSA_return_code _get_calendar_name(Calendar *cal, CSA_attribute *attr);
static CSA_return_code _get_product_identifier(Calendar *cal,
						CSA_attribute *attr);
static CSA_return_code _get_version_supported(Calendar *cal,
						CSA_attribute *attr);
static CSA_return_code _get_server_version(Calendar *cal, CSA_attribute *attr);
static CSA_return_code _get_data_version(Calendar *cal, CSA_attribute *attr);
static CSA_return_code _get_access_list(Calendar *cal, uint num_names,
					char ** names, CSA_attribute *attr);
static CSA_return_code _get_number_entries(Calendar *cal, uint num_names,
					char ** names, CSA_attribute *attr);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

/*
 * Allocate a calendar structure and initialize it with
 * the location, and name of the calendar.
 */
extern Calendar *
_DtCm_new_Calendar(const char *calendar)
{
	Calendar	*cal;
	int i;

	_DtCm_init_hash();

	if ((cal = (Calendar *)malloc(sizeof(Calendar))) == NULL)
		return(NULL);

	memset((void *)cal, NULL, sizeof(Calendar));

	if ((cal->name = strdup(calendar)) == NULL) {
		free(cal);
		return(NULL);
	}

	cal->location = strchr(cal->name, '@');
	cal->location++;

	/* set up attribute array */
	if ((cal->attrs = (cms_attribute *)calloc(1,
	    sizeof(cms_attribute) * (_DtCm_cal_name_tbl->size + 1))) == NULL) {
		free(cal->name);
		free(cal);
		return(NULL);
	}

	for (i = 1; i <= _DtCm_cal_name_tbl->size; i++) {
		if ((cal->attrs[i].name.name =
		    strdup(_DtCm_cal_name_tbl->names[i])) == NULL) {
			/* clean up */
			cal->num_attrs = i - 1;
			_DtCm_free_Calendar(cal);
			return(NULL);
		} else
			cal->attrs[i].name.num = i;
	}
	cal->num_attrs = _DtCm_cal_name_tbl->size;

	cal->cal_tbl = _DtCm_cal_name_tbl;
	cal->entry_tbl = _DtCm_entry_name_tbl;

	cal->handle = (CSA_session_handle)cal;

	return (cal);
}

extern void
_DtCm_put_calendar_in_list(Calendar *cal)
{
	/* link the new calendar to the list */
  	pthread_mutex_lock(&_DtCm_active_cal_list.lock);
	cal->next = _DtCm_active_cal_list.next;
	_DtCm_active_cal_list.next = cal;
	_DtCm_active_cal_list.access++;
	pthread_mutex_unlock(&_DtCm_active_cal_list.lock);
}

/*
 * After freeing up memory pointed to by cal,
 * put it in the free list.
 */
extern void
_DtCm_free_Calendar(Calendar *cal)
{
	_DtCmCallbackEntry	*cb, *ptr;

	/* free up resources used by it */
	if (cal->name) free(cal->name);

	if (cal->cal_tbl != _DtCm_cal_name_tbl)
		_DtCm_free_name_table(cal->cal_tbl);

	if (cal->entry_tbl != _DtCm_entry_name_tbl)
		_DtCm_free_name_table(cal->entry_tbl);

	cb = cal->cb_list;
	while (cb) {
		ptr = cb->next;
		free(cb);
		cb = ptr;
	}

	/*
	 * no need to free attribute values now
	 * since the values are passed to client
	 * directly.
	 * need to free attribute values if they
	 * are cached later
	 */
	_DtCm_free_cms_attribute_values(cal->num_attrs, cal->attrs);
	free(cal->attrs);

	memset((void *)cal, '\0', sizeof(Calendar));
}

extern void
_DtCm_remove_calendar_from_list(Calendar *cal)
{
	Calendar		*prev, *ptr;

	/* remove the record from the active list */
	prev = &_DtCm_active_cal_list;
	pthread_mutex_lock(&prev->lock);
	_DtCm_active_cal_list.access--;
	while ((ptr = prev->next) != NULL) {
	  	pthread_mutex_lock(&ptr->lock);

		if (ptr == cal) {
			prev->next = ptr->next;
			pthread_mutex_unlock(&ptr->lock); 
			break;
		} else {
			pthread_mutex_unlock(&prev->lock);
			prev = ptr;
		}
	}
	pthread_mutex_unlock(&prev->lock);

	/* if no calendar is active, destroy rpc mapping */
	pthread_mutex_lock(&_DtCm_active_cal_list.lock);

	if ((_DtCm_active_cal_list.access) == 0) {
		/* if no calendar is open, destroy rpc mapping */
#ifdef CM_DEBUG
		_DtCm_print_host_list();
#endif
		_DtCm_destroy_agent();
	}

	pthread_mutex_unlock(&_DtCm_active_cal_list.lock);

	/* now take care of the to be deleted record */
	/* if other threads are blocking on this calendar,
	 * unblock them by doing cond_broadcast;
	 * otherwise, just free up the record
	 */
	pthread_mutex_lock(&cal->slock.lock);

	cal->handle = NULL;		/* invalidate the record */
	cal->slock.owner = NULL;	/* break loops for threads waiting
					 * on this record
					 */

	if (cal->slock.owner_count > 0) {
		cal->slock.owner_count--;
		pthread_mutex_unlock(&cal->slock.lock);
	} else if (cal->slock.wait_count > 0) {
		/* the record will be freed up by the last thread
		 * waiting on it
		 */
	  	pthread_cond_broadcast(&cal->slock.my_turn);
		pthread_mutex_unlock(&cal->slock.lock);
	} else {
		/* nobody's waiting, free up the record */
		pthread_mutex_unlock(&cal->slock.lock);
		_DtCm_free_Calendar(cal);
	}

}

/*
 * Given the calendar handle, return the internal calendar data structure.
 * The function will acquire the session lock so that only one thread
 * is active at anytime.
 * If other thread has the session lock, this thread will block.
 */
extern CSA_return_code
_DtCm_get_calendar(
	CSA_session_handle calhandle,
	boolean_t not_again,
	Calendar **calptr)
{
	Calendar	*cal = (Calendar *)calhandle;

	CSA_return_code	stat;
	Calendar	*prev, *ptr;

	if (cal == NULL)
		return (CSA_E_INVALID_SESSION_HANDLE);

	/* go through the list to find the calendar
	 * and lock the calendar session
	 */
	prev = &_DtCm_active_cal_list;
	pthread_mutex_lock(&prev->lock);
	while ((ptr = prev->next) != NULL) {
		pthread_mutex_lock(&ptr->lock);

		if (ptr == cal) {
			/* _DtCm_lock_session will unlock the access lock
			 * of cal and prev
			 */
			if ((stat = _DtCm_lock_session(cal, prev, not_again))
			    == CSA_SUCCESS)
				*calptr = cal;

			return (stat);
		}

		pthread_mutex_unlock(&prev->lock);
		prev = ptr;
	}

	pthread_mutex_unlock(&prev->lock);

	/* not found */
	return (CSA_E_INVALID_SESSION_HANDLE);

}

/*
 * Release the session lock.
 * If count is greater than zero, decrement the count.
 * Otherwise, set the owner to NULL and wait up one thread (if there's any)
 * waiting on my_turn.
 */
extern void
_DtCm_release_calendar(Calendar *cal)
{
	boolean_t	free = _B_FALSE;

	pthread_mutex_lock(&cal->slock.lock);

	if (cal->slock.owner_count > 0) {
		cal->slock.owner_count--;
	} else {
		if (cal->handle == (CSA_session_handle)cal) {
			cal->slock.owner = NULL;
			pthread_cond_signal(&cal->slock.my_turn);
		} else {
			/* calendar is logged off */
			if (cal->slock.wait_count > 0)
				pthread_cond_broadcast(&cal->slock.my_turn);
			else
				free = _B_TRUE;
		}
	}

	pthread_mutex_unlock(&cal->slock.lock);

	if (free == _B_TRUE)
		_DtCm_free_Calendar(cal);
	DP(("%d: unlock calendar\n", pthread_self()));
}

extern void
_DtCm_reset_cal_attrs(Calendar *cal)
{
	cal->got_attrs = _B_FALSE;
}

/*
 * Assume good parameters.  Caller should check before calling this.
 */
extern CSA_return_code
_DtCm_list_old_cal_attr_names(
	Calendar *cal,
	CSA_uint32 *num_names_r,
	char **names_r[])
{
	CSA_return_code	stat;
	char		**names, buf[BUFSIZ];
	int		i, j;

	if ((names = _DtCm_alloc_character_pointers(_DtCM_OLD_CAL_ATTR_SIZE))
	    == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* find out whether we know the owner of the calendar */
	if ((stat = _get_owner_from_old_cal(cal, buf)) != CSA_SUCCESS)
		return (stat);

	for (i = 1, j = 0; i <= _DtCM_DEFINED_CAL_ATTR_SIZE; i++) {
		if (_CSA_cal_attr_info[i].fst_vers > 0 &&
		    _CSA_cal_attr_info[i].fst_vers <= cal->file_version) {
			if (i == CSA_CAL_ATTR_CALENDAR_OWNER_I && *buf == NULL)
				continue;

			if ((names[j] =
			    strdup(_CSA_CALENDAR_ATTRIBUTE_NAMES[i])) == NULL)
			{
				_DtCm_free(names);
				return (CSA_E_INSUFFICIENT_MEMORY);
			} else
				j++;
		}
	}

	*names_r = names;
	*num_names_r = j;

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCm_get_all_cal_attrs(
	Calendar *cal,
	CSA_uint32 *num_attrs,
	CSA_attribute **attrs)
{
	CSA_return_code	stat;
	int		i, j;
	CSA_attribute	*attrs_r;

	if (num_attrs == NULL || attrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if (cal->rpc_version >= _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION) {
		if ((stat = _DtCm_rpc_get_cal_attrs(cal, 0, 0, NULL))
		    != CSA_SUCCESS)
			return (stat);

		if ((attrs_r = _DtCm_alloc_attributes(cal->num_attrs)) == NULL)
			return (CSA_E_INSUFFICIENT_MEMORY);

		/* get attributes */
		for (i = 1, j = 0; i <= cal->num_attrs; i++) {
			if (cal->attrs[i].value != NULL) {

				if ((stat = _DtCm_cms2csa_attribute(
				    cal->attrs[i], &attrs_r[j])) != CSA_SUCCESS)
				{
					_DtCm_free(attrs_r);
					return (stat);
				} else
					j++;
			}
		}

		*num_attrs = j;
		*attrs = attrs_r;

		return (CSA_SUCCESS);
	} else
		return (_DtCm_get_cal_attrs_by_name(cal,
			_DtCM_DEFINED_CAL_ATTR_SIZE + 1,
			_CSA_CALENDAR_ATTRIBUTE_NAMES, num_attrs, attrs));
}

/*
 * If it's not found, the attribute value is set to NULL.
 */
extern CSA_return_code
_DtCm_get_cal_attrs_by_name(
	Calendar *cal,
	CSA_uint32      num_names,
	CSA_attribute_reference *names,
	CSA_uint32      *num_attrs,
	CSA_attribute **attrs)
{
	CSA_return_code	stat = CSA_SUCCESS;
	CSA_attribute *attrs_r;
	int i, j, index;

	if (num_attrs == 0 || attrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((attrs_r = _DtCm_alloc_attributes(num_names)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0, j = 0; i < num_names; i++) {
		if (names[i] == NULL)
			continue;

		index = _DtCm_get_index_from_table(cal->cal_tbl, names[i]);

		switch (index) {
		case CSA_X_DT_CAL_ATTR_SERVER_VERSION_I:

			stat = _get_server_version(cal, &attrs_r[j]);
			break;

		case CSA_X_DT_CAL_ATTR_DATA_VERSION_I:

			stat = _get_data_version(cal, &attrs_r[j]);
			break;

		case CSA_CAL_ATTR_ACCESS_LIST_I:

			stat = _get_access_list(cal, num_names, names,
				&attrs_r[j]);
			break;

		case CSA_CAL_ATTR_NUMBER_ENTRIES_I:

			stat = _get_number_entries(cal, num_names,
				names, &attrs_r[j]);
			break;

		case CSA_CAL_ATTR_CALENDAR_NAME_I:

			stat = _get_calendar_name(cal, &attrs_r[j]);
			break;

		case CSA_CAL_ATTR_CALENDAR_OWNER_I:

			stat = _get_calendar_owner(cal, num_names, names,
				&attrs_r[j]);
			break;

		case CSA_CAL_ATTR_PRODUCT_IDENTIFIER_I:

			stat = _get_product_identifier(cal, &attrs_r[j]);
			break;

		case CSA_CAL_ATTR_VERSION_I:

			stat = _get_version_supported(cal, &attrs_r[j]);
			break;

		default:
			if (cal->rpc_version >=
			    _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION)
			{
				if (cal->file_version <
				    _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
				    index == -1)
					break;

				if (cal->got_attrs == _B_FALSE &&
				    (stat = _DtCm_rpc_get_cal_attrs(cal, 0,
				    num_names, names)) == CSA_SUCCESS) {
					/* there might by new attributes */
					if (index == -1 &&
					    (index = _DtCm_get_index_from_table(
					    cal->cal_tbl, names[i])) == -1)
						break;
				}

				if (stat == CSA_SUCCESS &&
				    cal->attrs[index].value)
					stat = _DtCm_cms2csa_attribute(
						cal->attrs[index], &attrs_r[j]);
			}
			break;
		}

		if (stat == CSA_SUCCESS) {
			if (attrs_r[j].value != NULL)
				j++;
		} else {
			_DtCm_free(attrs_r);
			return (stat);
		}
	}

	*num_attrs = j;

	if (j > 0) {
		*attrs = attrs_r;
	} else {
		*attrs = NULL;
		_DtCm_free(attrs_r);
	}

	return (CSA_SUCCESS);
}

/******************************************************************************
 * static functions used within in the file
 ******************************************************************************/

/*
 * Try to acquire the session lock.
 * If no other thread is holding the lock, acquire the lock by
 * setting the owner to the thread id.
 * Otherwise, if this thread already has the session, increment
 * the count.
 * Otherwise, some other thread has the session, wait on my_turn.
 * In any case, the access lock to the record will be released first.
 */
static CSA_return_code
_DtCm_lock_session(Calendar *cal, Calendar *prev, boolean_t not_again)
{
	CSA_return_code	stat = CSA_SUCCESS;
	int		free = _B_FALSE;
	pthread_t	me = pthread_self();

	pthread_mutex_lock(&cal->slock.lock);

	/* release the access lock to the list so that
	 * other threads can access other calendar sessions
	 */
	pthread_mutex_unlock(&cal->lock);
	pthread_mutex_unlock(&prev->lock);

	while (cal->slock.owner != NULL) {
		if (cal->slock.owner == me) {
		  if (not_again == _B_TRUE) {
		    stat = CSA_X_DT_E_MT_UNSAFE;
		  } else {
		    cal->slock.owner_count++;
		  }
		  pthread_mutex_unlock(&cal->slock.lock);
		  DP(("%d: lock calendar\n", me));
		  return (stat);
		} else {
			cal->slock.wait_count++;
			DP(("%d: tryint to lock calendar\n", me));
			pthread_cond_wait(&cal->slock.my_turn, &cal->slock.lock);
			cal->slock.wait_count--;
		}
	}

	if (cal->handle == (CSA_session_handle)cal) {
	  	cal->slock.owner = me;
		DP(("%d: lock calendar\n", me));
	} else {
		/* the session is logged off by others */
		stat = CSA_E_INVALID_SESSION_HANDLE;

		if (cal->slock.wait_count == 0)
			free = _B_TRUE;
	}

	pthread_mutex_unlock(&cal->slock.lock);
	
	if (free == _B_TRUE)
		_DtCm_free_Calendar(cal);

	return (stat);
}

/*
 * owner should point to a buffer big enough to hold the owner name
 * We test whether the calendar name is a user name, if so, the
 * owner will be the same as the calendar name.
 * Otherwise, we don't know the owner.
 */
static CSA_return_code
_get_owner_from_old_cal(Calendar *cal, char *owner)
{
	char		*calname;

	if ((calname = _DtCmGetPrefix(cal->name, '@')) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (_DtCmIsUserName(calname) == _B_TRUE)
		strcpy(owner, calname);
	else
		*owner = NULL;

	free(calname);
	return (CSA_SUCCESS);
}

static CSA_return_code
_get_calendar_owner(
	Calendar	*cal,
	uint		num_names,
	char		**names,
	CSA_attribute	*attr)
{
	CSA_return_code		stat;
	CSA_attribute_value	*val;
	char			*owner, buf[BUFSIZ];

	if (cal->rpc_version >= _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION) {
		if (cal->got_attrs == _B_FALSE) {
			if ((stat = _DtCm_rpc_get_cal_attrs(cal, 0, num_names,
			    names)) != CSA_SUCCESS)
				return (stat);
		}
		owner = cal->attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].value->\
			item.string_value;
	} else {
		if ((stat = _get_owner_from_old_cal(cal, buf)) != CSA_SUCCESS)
			return (stat);
		else if (*buf == NULL)
			return (CSA_SUCCESS);
		else
			owner = buf;
	}

	if (attr->name = strdup(CSA_CAL_ATTR_CALENDAR_OWNER)) {
		if ((val = (CSA_attribute_value *)calloc(1,
		    sizeof(CSA_attribute_value))) == NULL) {
			free(attr->name);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		val->type = CSA_VALUE_CALENDAR_USER;

		if ((val->item.calendar_user_value = (CSA_calendar_user *)
		    calloc(1, sizeof(CSA_calendar_user))) == NULL) {
			free(val);
			free(attr->name);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		if (val->item.calendar_user_value->user_name = strdup(owner)) {
			attr->value = val;
			return (CSA_SUCCESS);
		} else {
			free(val->item.calendar_user_value);
			free(val);
			free(attr->name);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
	} else
		return (CSA_E_INSUFFICIENT_MEMORY);
}

static CSA_return_code
_get_calendar_name(Calendar *cal, CSA_attribute *attr)
{
	if (attr->name = strdup(CSA_CAL_ATTR_CALENDAR_NAME))
		return (_DtCm_set_csa_string_attrval(cal->name, &attr->value,
			CSA_VALUE_STRING));
	else
		return (CSA_E_INSUFFICIENT_MEMORY);
}

static CSA_return_code
_get_product_identifier(Calendar *cal, CSA_attribute *attr)
{
	if (attr->name = strdup(CSA_CAL_ATTR_PRODUCT_IDENTIFIER))
		return (_DtCm_set_csa_string_attrval(_DtCM_PRODUCT_IDENTIFIER,
			&attr->value, CSA_VALUE_STRING));
	else
		return (CSA_E_INSUFFICIENT_MEMORY);
}

static CSA_return_code
_get_version_supported(Calendar *cal, CSA_attribute *attr)
{
	char buf[10];

	if (attr->name = strdup(CSA_CAL_ATTR_VERSION))
		return (_DtCm_set_csa_string_attrval(
			_DtCM_SPEC_VERSION_SUPPORTED, &attr->value,
			CSA_VALUE_STRING));
	else
		return (CSA_E_INSUFFICIENT_MEMORY);
}

static CSA_return_code
_get_server_version(Calendar *cal, CSA_attribute *attr)
{
	if (attr->name = strdup(CSA_X_DT_CAL_ATTR_SERVER_VERSION))
		return (_DtCm_set_csa_uint32_attrval(cal->rpc_version,
			&attr->value));
	else
		return (CSA_E_INSUFFICIENT_MEMORY);
}

static CSA_return_code
_get_data_version(Calendar *cal, CSA_attribute *attr)
{
	if (attr->name = strdup(CSA_X_DT_CAL_ATTR_DATA_VERSION))
		return (_DtCm_set_csa_uint32_attrval(cal->file_version,
			&attr->value));
	else
		return (CSA_E_INSUFFICIENT_MEMORY);
}

static CSA_return_code
_get_access_list(
	Calendar	*cal,
	uint		num_names,
	char		**names,
	CSA_attribute	*attr)
{
	CSA_return_code stat = CSA_SUCCESS;

	if (cal->rpc_version < _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION)
		stat = _DtCm_rpc_get_cal_attrs(cal, CSA_CAL_ATTR_ACCESS_LIST_I,
			0, NULL);
	else if (cal->got_attrs == _B_FALSE)
		stat = _DtCm_rpc_get_cal_attrs(cal, 0, num_names, names);

	if (stat == CSA_SUCCESS)
		stat = _DtCm_cms2csa_attribute(
			cal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I], attr);

	return (stat);
}

static CSA_return_code
_get_number_entries(
	Calendar	*cal,
	uint		num_names,
	char		**names,
	CSA_attribute	*attr)
{
	CSA_return_code stat = CSA_SUCCESS;

	if (cal->rpc_version < _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION)
		stat = _DtCm_rpc_get_cal_attrs(cal,
			CSA_CAL_ATTR_NUMBER_ENTRIES_I, 0, NULL);
	else if (cal->got_attrs == _B_FALSE)
		stat = _DtCm_rpc_get_cal_attrs(cal, 0, num_names, names);

	if (stat == CSA_SUCCESS)
		stat = _DtCm_cms2csa_attribute(
			cal->attrs[CSA_CAL_ATTR_NUMBER_ENTRIES_I], attr);

	return (stat);
}

