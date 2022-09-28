/* $XConsortium: lookup.c /main/cde1_maint/4 1995/10/10 13:34:16 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)lookup.c	1.14 96/11/12 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <values.h>
#ifdef SunOS
#include <sys/systeminfo.h>
#endif
#include "attr.h"
#include "cmscalendar.h"
#include "lookup.h"
#include "cm.h"
#include "tree.h"
#include "list.h"
#include "iso8601.h"
#include "rerule.h"
#include "reutil.h"
#include "access.h"
#include "cmsmatch.h"
#include "cmsdata.h"
#include "cmsentry.h"
#include "repeat.h"
#include "v5ops.h"
#include "misc.h"

#define TIME_BUF_LEN	20

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
static CSA_return_code _AddToLinkedEntries(cms_entry *eptr, cms_entry **head,
					cms_entry **tail, boolean_t sort,
					boolean_t timeonly);
static CSA_return_code _EnumerateSequence(char *sender, CSA_flags access,
					List_node *lnode, time_t start1,
					time_t start2,
					boolean_t no_end_time_range,
					time_t end1, time_t end2,
					CSA_uint32 num_attrs,
					cms_attribute *attrs, CSA_enum *ops,
					cms_entry **head, cms_entry **tail);
static CSA_return_code _GetEntryAttrsByName(_DtCmsCalendar *cal,
					cms_entry *entry, CSA_uint32 num_names,
					cms_attr_name *names,
					CSA_uint32 *num_attrs_r,
					cms_attribute **attrs_r);
static CSA_return_code _GetAllEntryAttrs(cms_entry *entry,
					CSA_uint32 *num_attrs_r,
					cms_attribute **attrs_r);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

extern CSA_return_code
_DtCmsLookupEntriesById(
	_DtCmsCalendar	*cal,
	char		*sender,
	CSA_flags	access,
	boolean_t	no_start_time_range,
	boolean_t	no_end_time_range,
	time_t		start1,
	time_t		start2,
	time_t		end1,
	time_t		end2,
	long		id,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	CSA_enum	*ops,
	cms_entry	**entries)
{
	CSA_return_code	stat = CSA_SUCCESS;
	cms_entry	*eptr, *head = NULL, *tail = NULL;
	cms_key		key;
	time_t		endtick;
	cms_attribute	*aptr;
	boolean_t	timeonly;

	if (no_start_time_range) {
		start1 = _DtCM_BOT;
		start2 = _DtCM_EOT;
	}

	/* do lookup on repeating entries first */	
	stat = _DtCmsEnumerateSequenceById(cal, sender, access,
		no_start_time_range, no_end_time_range, start1, start2,
		end1, end2, id, num_attrs, attrs, ops, &head);
	
	if (stat != CSA_X_DT_E_ENTRY_NOT_FOUND) {
		if (stat == CSA_SUCCESS)
			*entries = head;
		return (stat);
	}

	key.time = start1;
	key.id = id;
	*entries = NULL;
	while ((eptr = (cms_entry *)rb_lookup_next_larger(cal->tree,
	    (caddr_t)&key)) && eptr->key.time < start2) {
		if (eptr->key.id != id) {
			key.time = eptr->key.time;
			key.id = eptr->key.id;
		} else {
			aptr = &eptr->attrs[CSA_ENTRY_ATTR_END_DATE_I];
			if (!no_end_time_range) {
				if (aptr->value == NULL)
					return (CSA_SUCCESS);

				_csa_iso8601_to_tick(
					aptr->value->item.date_time_value,
					&endtick);

				if (endtick <= end1 || endtick >= end2)
					return (CSA_SUCCESS);
			}
			
			if (_DtCmsMatchAttributes(eptr, num_attrs, attrs, ops)
			    && (_DtCmsCheckViewAccess(sender, access, eptr,
			    &timeonly) == CSA_SUCCESS)) {
				stat = _AddToLinkedEntries(eptr, &head, &tail,
					_B_FALSE, timeonly);
			}
			*entries = head;
			return (stat);
		}
	}

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmsLookupEntries(
	_DtCmsCalendar	*cal,
	char		*sender,
	CSA_flags	access,
	time_t		start1,
	time_t		start2,
	boolean_t	no_end_time_range,
	time_t		end1,
	time_t		end2,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	CSA_enum	*ops,
	cms_entry	**entries)
{
	CSA_return_code	stat = CSA_SUCCESS;
	cms_entry	*eptr, *head = NULL, *tail = NULL;
	cms_key		key;
	List_node	*lnode;
	time_t		endtick;
	cms_attribute	*aptr;
	boolean_t	timeonly;

	/* do lookup on one-time entries first */
	key.time = start1;
	key.id = 0;
	while ((eptr = (cms_entry *)rb_lookup_next_larger(cal->tree,
	    (caddr_t)&key)) && eptr->key.time < start2) {

		aptr = &eptr->attrs[CSA_ENTRY_ATTR_END_DATE_I];

		if (!no_end_time_range) {
			if (aptr->value == NULL)
				goto nextone;

			_csa_iso8601_to_tick(aptr->value->item.date_time_value,
				&endtick);

			if (endtick <= end1 || endtick >= end2)
				goto nextone;
		}

		if (_DtCmsMatchAttributes(eptr, num_attrs, attrs, ops) &&
		    _DtCmsCheckViewAccess(sender, access, eptr, &timeonly)
		    == CSA_SUCCESS)
		{
			/* the last argument "sort" is set to _B_FALSE,
			 * because the entries are in order already
			 * as we get them out from the tree;
			 * we don't want to sort it again
			 */
			if ((stat = _AddToLinkedEntries(eptr, &head, &tail,
			    _B_FALSE, timeonly)) != CSA_SUCCESS)
				break;
		}

nextone:
		key.time = eptr->key.time;
		key.id = eptr->key.id;
	}

	/* do lookup on repeating entries */	
	lnode = cal->list->root;
	while (lnode != NULL && stat == CSA_SUCCESS) {

		stat = _EnumerateSequence(sender, access, lnode, start1, start2,
			no_end_time_range, end1, end2, num_attrs, attrs,
			ops, &head, &tail);

		lnode = hc_lookup_next(lnode);
	}

	if (stat == CSA_SUCCESS)
		*entries = head;
	else if (head)
		_DtCm_free_cms_entries(head);

	return (stat);
}

extern CSA_return_code
_DtCmsEnumerateSequenceById(
	_DtCmsCalendar	*cal,
	char		*sender,
	CSA_flags	access,
	boolean_t	no_start_time_range,
	boolean_t	no_end_time_range,
	time_t		start1,
	time_t		start2,
	time_t		end1,
	time_t		end2,
	long		id,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	CSA_enum	*ops,
	cms_entry	**entries)
{
	CSA_return_code	stat;
	cms_entry	*head = NULL, *tail = NULL;
	cms_key		key;
	List_node	*lnode;
	boolean_t	timeonly;

	key.id = id;
	lnode = hc_lookup_node(cal->list, (caddr_t)&key);
	if (lnode == NULL)
		return (CSA_X_DT_E_ENTRY_NOT_FOUND);

	*entries = NULL;
	if (no_start_time_range && no_end_time_range) {
		if ((stat = _DtCmsCheckViewAccess(sender, access,
		    (cms_entry *)lnode->data, &timeonly)) != CSA_SUCCESS)
			return (stat);

		if (_DtCmsMatchAttributes((cms_entry *)lnode->data, num_attrs,
		    attrs, ops)) {
			stat = _DtCmsGetCmsEntryForClient(
				(cms_entry *)lnode->data, entries, timeonly);
		}
	} else {
		stat = _EnumerateSequence(sender, access, lnode, start1, start2,
			no_end_time_range, end1, end2, num_attrs, attrs, ops,
			&head, &tail);

		if (stat == CSA_SUCCESS)
			*entries = head;
		else if (head)
			_DtCm_free_cms_entries(head);
	}

	return (stat);
}

extern CSA_return_code
_DtCmsLookupEntriesByKey(
	_DtCmsCalendar	*cal,
	char		*user,
	CSA_flags	access,
	CSA_uint32	num_keys,
	cms_key		*keys,
	CSA_uint32	num_names,
	cms_attr_name	*names,
	cms_get_entry_attr_res_item **res)
{
	CSA_return_code			stat = CSA_SUCCESS;
	cms_get_entry_attr_res_item	*eptr, *head;
	int				i;

	for (i = 0, head = NULL; i < num_keys; i++) {
		if ((stat = _DtCmsGetEntryAttrByKey(cal, user, access,
		    keys[i], num_names, names, NULL, &eptr)) == CSA_SUCCESS) {
			eptr->next = head;
			head = eptr;
		} else {
			if (head) _DtCmsFreeEntryAttrResItem(head);
			return (stat);
		}
	}

	*res = head;
	return (CSA_SUCCESS);
}

/*
 * this routine returns either a cms_entry or 
 * an cms_get_entry_attr_res_item structure
 * depending which output argument is not null
 */
extern CSA_return_code
_DtCmsGetEntryAttrByKey(
	_DtCmsCalendar	*cal,
	char		*user,
	CSA_flags	access,
	cms_key		key,
	CSA_uint32	num_names,
	cms_attr_name	*names,
	cms_entry	**entry_r,
	cms_get_entry_attr_res_item **res_r)
{
	CSA_return_code	stat;
	cms_entry	*entry = NULL;
	char		*stime, *etime;
	char		sbuf[TIME_BUF_LEN], ebuf[TIME_BUF_LEN];
	time_t		firsttick = 0;
	List_node	*lnode;
	boolean_t	timeonly;
	char		*summary;
	cms_attribute_value *vptr;
	cms_get_entry_attr_res_item *res;

	if (entry_r == NULL && res_r == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if (res_r && (res = (cms_get_entry_attr_res_item *)calloc(1,
	    sizeof(cms_get_entry_attr_res_item))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* do lookup on one-time entries first */
	if ((entry = (cms_entry *)rb_lookup(cal->tree, (caddr_t)&key)) == NULL)
	{
		if ((lnode = (List_node *)hc_lookup_node(cal->list,
		    (caddr_t)&key)) != NULL)
		{
			entry = (cms_entry *)lnode->data;
			if (_DtCmsInExceptionList(entry, key.time))
				entry = NULL;
		}
	}

	if (entry == NULL ||
	    _DtCmsCheckViewAccess(user, access, entry, &timeonly)) {
		if (entry == NULL)
			stat = CSA_X_DT_E_ENTRY_NOT_FOUND;
		else
			stat = CSA_E_NO_AUTHORITY;

		if (entry_r)
			return (stat);
		else {
			res->stat = stat;
			*res_r = res;
			return (CSA_SUCCESS);
		}
	}

	if (entry->key.time != key.time) {
		/* set start and end time of the instance */
		firsttick = entry->key.time;
		entry->key.time = key.time;

		stime = entry->attrs[CSA_ENTRY_ATTR_START_DATE_I].value->\
			item.date_time_value;
		_csa_tick_to_iso8601(key.time, sbuf);
		entry->attrs[CSA_ENTRY_ATTR_START_DATE_I].value->\
			item.date_time_value = sbuf;

		if (entry->attrs[CSA_ENTRY_ATTR_END_DATE_I].value) {
			etime = entry->attrs[CSA_ENTRY_ATTR_END_DATE_I].\
				value->item.date_time_value;
			_csa_tick_to_iso8601(key.time + lnode->duration, ebuf);
			entry->attrs[CSA_ENTRY_ATTR_END_DATE_I].value->\
				item.date_time_value = ebuf;
		}
	}

	if (entry_r)
		stat = _DtCmsGetCmsEntryForClient(entry, entry_r, timeonly);
	else {

		if (timeonly == _B_TRUE) {
			/* replace the content of the summary attribute to
			 * a canned string
			 */
			vptr = entry->attrs[CSA_ENTRY_ATTR_SUMMARY_I].value;
			summary = vptr->item.string_value;

			switch (entry->attrs[CSA_ENTRY_ATTR_TYPE_I].value->\
				item.uint32_value) {
			case CSA_TYPE_EVENT:
				vptr->item.string_value = _DtCM_APPOINTMENT_STRING;
				break;
			case CSA_TYPE_TODO:
				vptr->item.string_value = _DtCM_TODO_STRING;
				break;
			case CSA_TYPE_MEMO:
				vptr->item.string_value = _DtCM_MEMO_STRING;
				break;
			default:
				vptr->item.string_value = _DtCM_ENTRY_STRING;
				break;
			}
		}

		if (num_names == 0)
			stat = _GetAllEntryAttrs(entry, &res->num_attrs,
				&res->attrs);
		else
			stat = _GetEntryAttrsByName(cal, entry, num_names,
				names, &res->num_attrs, &res->attrs);

		if (timeonly == _B_TRUE) {
			vptr->item.string_value = summary;
		}

		if (stat == CSA_SUCCESS) {
			res->key = key;
			*res_r = res;
		} else
			free(res);
	}

	if (firsttick > 0) {
		entry->key.time = firsttick;
		entry->attrs[CSA_ENTRY_ATTR_START_DATE_I].value->\
			item.date_time_value = stime;
		if (entry->attrs[CSA_ENTRY_ATTR_END_DATE_I].value)
			entry->attrs[CSA_ENTRY_ATTR_END_DATE_I].value->\
				item.date_time_value = etime;
	}

	return (stat);
}

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

static CSA_return_code
_AddToLinkedEntries(
	cms_entry	*eptr,
	cms_entry	**head,
	cms_entry	**tail,
	boolean_t	sort,
	boolean_t	timeonly)
{
	CSA_return_code	stat;
	cms_entry	*newptr, *prev, *ptr;

	if ((stat = _DtCmsGetCmsEntryForClient(eptr, &newptr, timeonly))
	    != CSA_SUCCESS)
		return (stat);

	if (sort == _B_FALSE) {
		/* just add to the end of the list */

		if ((*head) == NULL)
			*head = newptr;
		else
			(*tail)->next = newptr;

		*tail = newptr;
	} else {
		/* add item in ascending order */
		for (prev = NULL, ptr = *head; ptr != NULL;
		    prev = ptr, ptr = ptr->next) {
			if (eptr->key.time <= ptr->key.time)
				break;
		}

		newptr->next = ptr;
		if (prev == NULL)
			*head = newptr;
		else
			prev->next = newptr;

		if ((*tail) == prev)
			*tail = newptr;
	}

	return (CSA_SUCCESS);
}

static CSA_return_code
_EnumerateSequence(
	char		*sender,
	CSA_flags	access,
	List_node	*lnode,
	time_t		start1,
	time_t		start2,
	boolean_t	no_end_time_range,
	time_t		end1,
	time_t		end2,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	CSA_enum	*ops,
	cms_entry	**head,
	cms_entry	**tail)
{
	CSA_return_code	stat = CSA_SUCCESS;
	cms_entry	*eptr;
	long		fsttick, tick;
	boolean_t	timeonly;
	RepeatEventState *restate;

	eptr = (cms_entry *)lnode->data;
	fsttick = eptr->key.time;

	if (lnode->lasttick == 0) {
		lnode->lasttick = LastTick(fsttick, lnode->re);
		lnode->duration = _DtCmsGetDuration(eptr);
	}

	if (lnode->lasttick <= start1 || fsttick >= start2 ||
	    (!no_end_time_range &&
	     eptr->attrs[CSA_ENTRY_ATTR_END_DATE_I].value == NULL) ||
	    (!no_end_time_range && (lnode->lasttick+lnode->duration) <= end1) ||
	    (!no_end_time_range && (fsttick + lnode->duration) >= end2) ||
	    _DtCmsCheckViewAccess(sender, access, eptr, &timeonly) ||
	    !_DtCmsMatchAttributes(eptr, num_attrs, attrs, ops))
		return (CSA_SUCCESS);

	if (!no_end_time_range && start1 < (end1 - lnode->duration))
		start1 = end1 - lnode->duration;

	if (!no_end_time_range && ((end2 - lnode->duration) < start2))
		start2 = end2 - lnode->duration;

	for (tick = ClosestTick(start1, fsttick, lnode->re, &restate);
	    stat == CSA_SUCCESS && tick < start2;
	    tick = NextTick(tick, fsttick, lnode->re, restate))
	{
		char *stime, *etime;
		char sbuf[TIME_BUF_LEN], ebuf[TIME_BUF_LEN];

		if (tick <= 0 || tick > lnode->lasttick)
			break;

		if (tick <= start1 || _DtCmsInExceptionList(eptr, tick))
			continue;

		/* set start and end time of the instance */
		eptr->key.time = tick;

		stime = eptr->attrs[CSA_ENTRY_ATTR_START_DATE_I].value->\
			item.date_time_value;
		_csa_tick_to_iso8601(tick, sbuf);
		eptr->attrs[CSA_ENTRY_ATTR_START_DATE_I].value->\
			item.date_time_value = sbuf;

		if (eptr->attrs[CSA_ENTRY_ATTR_END_DATE_I].value) {
			etime = eptr->attrs[CSA_ENTRY_ATTR_END_DATE_I].\
				value->item.date_time_value;
			_csa_tick_to_iso8601(tick + lnode->duration, ebuf);
			eptr->attrs[CSA_ENTRY_ATTR_END_DATE_I].value->\
				item.date_time_value = ebuf;
		}

		stat = _AddToLinkedEntries(eptr, head, tail, _B_TRUE, timeonly);

		eptr->key.time = fsttick;
		eptr->attrs[CSA_ENTRY_ATTR_START_DATE_I].value->\
			item.date_time_value = stime;
		if (eptr->attrs[CSA_ENTRY_ATTR_END_DATE_I].value)
			eptr->attrs[CSA_ENTRY_ATTR_END_DATE_I].value->\
				item.date_time_value = etime;
	}

	return (stat);
}

static CSA_return_code
_GetEntryAttrsByName(
	_DtCmsCalendar	*cal,
	cms_entry	*entry,
	CSA_uint32	num_names,
	cms_attr_name	*names,
	CSA_uint32	*num_attrs_r,
	cms_attribute	**attrs_r)
{
	CSA_return_code	stat = CSA_SUCCESS;
	cms_attribute	*attrs;
	int		i;

	if ((attrs = calloc(1, sizeof(cms_attribute)*num_names)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* return all attrs names with hash number
	 * so that caller knows which one does not
	 * have value and which one does not even exist
	 */
	for (i = 0; i < num_names && stat == CSA_SUCCESS; i++) {
		if (names[i].name == NULL)
			continue;

		if (names[i].num <= 0)
			names[i].num = _DtCm_get_index_from_table(
					cal->entry_tbl, names[i].name);

		if (names[i].num > 0) {
			stat = _DtCm_copy_cms_attribute(&attrs[i],
			    &entry->attrs[names[i].num], _B_TRUE);
		} else {
			attrs[i].name.num = -1;
			if ((attrs[i].name.name = strdup(names[i].name))==NULL)
				stat = CSA_E_INSUFFICIENT_MEMORY;
		}
	}

	if (stat == CSA_SUCCESS) {
		*num_attrs_r = num_names;
		*attrs_r = attrs;
	} else {
		_DtCm_free_cms_attributes(i, attrs);
		free(attrs);
	}

	return (stat);
}

static CSA_return_code
_GetAllEntryAttrs(
	cms_entry	*entry,
	CSA_uint32	*num_attrs_r,
	cms_attribute	**attrs_r)
{
	CSA_return_code	stat = CSA_SUCCESS;
	cms_attribute	*attrs;
	int		i, j;

	if ((attrs = calloc(1, sizeof(cms_attribute)*entry->num_attrs)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* first element is not used */
	for (i = 1, j = 0; i <= entry->num_attrs && stat == CSA_SUCCESS; i++) {
		if (entry->attrs[i].value) {
			if ((stat = _DtCm_copy_cms_attribute(&attrs[j],
			    &entry->attrs[i], _B_TRUE))
			    == CSA_SUCCESS)
			{
				j++;
			}
		}
	}

	if (stat == CSA_SUCCESS) {
		if (j > 0) {
			*num_attrs_r = j;
			*attrs_r = attrs;
		} else {
			*num_attrs_r = 0;
			*attrs_r = NULL;
			free(attrs);
		}
	} else {
		_DtCm_free_cms_attributes(j, attrs);
		free(attrs);
	}

	return (stat);
}


