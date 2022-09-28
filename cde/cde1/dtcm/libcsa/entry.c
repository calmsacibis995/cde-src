/* $XConsortium: entry.c /main/cde1_maint/3 1995/10/10 13:29:24 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)entry.c	1.22 96/11/12 Sun Microsystems, Inc."

/*
 * Functions that manage the entry data structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appt4.h"
#include "attr.h"
#include "calendar.h"
#include "cmsdata.h"
#include "convert4-5.h"
#include "convert5-4.h"
#include "debug.h"
#include "entry.h"
#include "free.h"
#include "nametbl.h"
#include "rpccalls.h"
#include "iso8601.h"
#include "updateattrs.h"

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
static CSA_return_code _CmsentryToLibentry(_DtCmNameTable **tbl, cms_entry *e,
			_DtCm_libentry	**entry_r);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

/*
 * Given the entry handle, return the internal entry data structure.
 */
extern _DtCm_libentry *
_DtCm_get_libentry(CSA_entry_handle entryhandle)
{
	_DtCm_libentry	*entry = (_DtCm_libentry *)entryhandle;

	if (entry == NULL || entry->handle != (void *)entry)
		return (NULL);
	else
		return (entry);
}

extern CSA_return_code
_DtCm_make_libentry(cms_entry *e, _DtCm_libentry **entry_r)
{
	_DtCm_libentry	*ptr;
	CSA_return_code	stat = CSA_SUCCESS;

	if (entry_r == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((ptr = (_DtCm_libentry *)calloc(1, sizeof(_DtCm_libentry)))
	    == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (e == NULL) {
		if ((ptr->e = _DtCm_make_cms_entry(_DtCm_entry_name_tbl))
		    == NULL)
			stat = CSA_E_INSUFFICIENT_MEMORY;
	} else
		stat = _DtCm_copy_cms_entry(e, &ptr->e);

	if (stat == CSA_SUCCESS) {
		ptr->handle = (void *)ptr;
		*entry_r = ptr;
	} else
		free(ptr);

	return (stat);
}

/*
 * return attribute names of all attributes.
 * ** how to deal with
 *	- predefined attributes with null values
 *	- custom attributes
 */
extern CSA_return_code
_DtCm_get_entry_attr_names(
	_DtCm_libentry *entry,
	CSA_uint32 *num_names_r,
	char **names_r[])
{
	char	**names;
	int	i, j;

	*names_r = NULL;
	*num_names_r = 0;

	if ((names = _DtCm_alloc_character_pointers(entry->e->num_attrs))
	    == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* first element in attr array is not used */
	for (i = 1, j = 0; i <= entry->e->num_attrs; i++) {
		/* there should not be any NULL names in the attr array */
		if (entry->e->attrs[i].value != NULL) {
			if ((names[j] = strdup(entry->e->attrs[i].name.name))
			    == NULL) {
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
_DtCm_get_all_entry_attrs(
	_DtCm_libentry *entry,
	CSA_uint32 *num_attrs,
	CSA_attribute **attrs)
{
	int		i, j;
	CSA_return_code	stat = CSA_SUCCESS;
	CSA_attribute	*attrs_r;

	if (num_attrs == NULL || attrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((attrs_r = _DtCm_alloc_attributes(entry->e->num_attrs)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* first element in attr array is not used */
	for (i = 1, j = 0; i <= entry->e->num_attrs; i++) {
		if (entry->e->attrs[i].value != NULL) {

			if ((stat = _DtCm_cms2csa_attribute(entry->e->attrs[i],
			    &attrs_r[j])) != CSA_SUCCESS) {
				_DtCm_free(attrs_r);
				return (stat);
			} else
				j++;
		}
	}

	*num_attrs = j;
	*attrs = attrs_r;

	return (CSA_SUCCESS);
}

/*
 * Search the attribute list for the given attribute names.
 * If it's not found, the attribute value
 * is set to NULL.
 */
extern CSA_return_code
_DtCm_get_entry_attrs_by_name(
	_DtCm_libentry *entry,
	CSA_uint32 num_names,
	CSA_attribute_reference *names,
	CSA_uint32 *num_attrs,
	CSA_attribute **attrs)
{
	int		i, j, index;
	CSA_return_code	stat = CSA_SUCCESS;
	CSA_attribute	*attrs_r;

	if (num_attrs == NULL || attrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((attrs_r = _DtCm_alloc_attributes(num_names)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* get attributes */
	for (i = 0, j = 0; i < num_names; i++) {
		if (names[i] != NULL) {
			index = _DtCm_get_index_from_table(
				entry->cal->entry_tbl, names[i]);

			if (index >= 0 && entry->e->attrs[index].value) {
				if (attrs_r[j].name =
				    strdup(entry->e->attrs[index].name.name))
				{
					stat = _DtCm_cms2csa_attrval(
						entry->e->attrs[index].value,
						&attrs_r[j].value);
				} else
					stat = CSA_E_INSUFFICIENT_MEMORY;

				if (stat != CSA_SUCCESS) {
					_DtCm_free(attrs_r);
					return (stat);
				} else
					j++;
			}
		}
	}

	*num_attrs = j;
	*attrs = attrs_r;

	return (CSA_SUCCESS);
}

/*
 * convert the linked list of entry structures to
 * an array of entry handles.
 */
extern CSA_return_code
_DtCm_libentry_to_entryh(
	_DtCm_libentry *elist,
	CSA_uint32 *size,
	CSA_entry_handle **entries_r)
{
	CSA_entry_handle	*eh;
	_DtCm_libentry	*ptr;
	int		i, j;

	if (elist == NULL || size == NULL || entries_r == NULL)
		return (CSA_E_INVALID_PARAMETER);

	for (i = 0, ptr = elist; ptr != NULL; ptr = ptr->next)
		i++;

	if ((eh = _DtCm_alloc_entry_handles(i)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (j = 0; j < i; j++, elist = elist->next)
		eh[j] = (CSA_entry_handle)elist;

	*size = i;
	*entries_r = eh;

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmCmsentriesToLibentries(
	_DtCmNameTable	**tbl,
	cms_entry	*entries,
	_DtCm_libentry	**libentries)
{
	CSA_return_code	stat = CSA_SUCCESS;
        _DtCm_libentry	*entry, *head, *prev;

	if (libentries == NULL)
		return(CSA_E_INVALID_PARAMETER);

	prev = head = NULL;
	while (entries != NULL) {

		if ((stat = _CmsentryToLibentry(tbl, entries, &entry))
		    != CSA_SUCCESS)
			break;

		if (head == NULL)
			head = entry;
		else {
			prev->next = entry;
			entry->prev = prev;
		}

		prev = entry;

		entries = entries->next;
	}

	if (stat != CSA_SUCCESS) {
		_DtCm_free_libentries(head);
		head = NULL;
	}

	*libentries = head;
	return(stat);
}

extern CSA_return_code
_DtCm_appt4_to_libentries(
	char		*calname,
	Appt_4		*appt4,
	_DtCm_libentry	**libentries)
{
	CSA_return_code	stat = CSA_SUCCESS;
        _DtCm_libentry	*entry, *head, *prev;

	if (libentries == NULL)
		return(CSA_E_INVALID_PARAMETER);

	prev = head = NULL;
	while (appt4 != NULL) {

		if ((stat = _DtCm_make_libentry(NULL, &entry)) != CSA_SUCCESS)
			break;

		if ((stat = _DtCm_appt4_to_attrs(calname, appt4,
		    entry->e->num_attrs, entry->e->attrs, _B_FALSE))
		    != CSA_SUCCESS)
			break;

		entry->e->key.time = appt4->appt_id.tick;
		entry->e->key.id = appt4->appt_id.key;
		
		if (head == NULL)
			head = entry;
		else {
			prev->next = entry;
			entry->prev = prev;
		}

		prev = entry;

		appt4 = appt4->next;
	}

	if (stat != CSA_SUCCESS) {
		_DtCm_free_libentries(head);
		head = NULL;
	}

	*libentries = head;
	return(stat);
}

extern CSA_return_code
_DtCm_libentries_to_appt4(_DtCm_libentry *entries, Appt_4 **appt4)
{
	CSA_return_code	stat = CSA_SUCCESS;
	Appt_4		*a4, *head, *prev;

	if (appt4 == NULL)
		return(CSA_E_INVALID_PARAMETER);

	prev = head = NULL;
	while (entries != NULL) {

		if ((stat = _DtCm_cms_entry_to_appt4(entries->e, &a4)) != CSA_SUCCESS)
			break;

		if (head == NULL)
			head = a4;
		else {
			prev->next = a4;
		}

		prev = a4;

		entries = entries->next;
	}

	if (stat != CSA_SUCCESS) {
		_DtCm_free_appt4(head);
		head = NULL;
	}

	*appt4 = head;
	return(stat);
}

extern CSA_return_code
_DtCm_reminder4_to_csareminder_detail(
	char			*calname,
	Reminder_4		*r4,
	Appt_4			*appts,
	CSA_uint32		*num_rems,
	CSA_reminder_reference	**rems)
{
	CSA_return_code		stat = CSA_SUCCESS;
	_DtCm_libentry		*entry;
        CSA_reminder_reference	*rem_r;
	int			i, count;
	Reminder_4		*rptr;
	Appt_4			*aptr;
	char			isotime[BUFSIZ];

	if (num_rems == NULL || rems == NULL)
		return(CSA_E_INVALID_PARAMETER);

	if (r4 == NULL || appts == NULL) {
		*num_rems = 0;
		*rems = NULL;
		return (CSA_SUCCESS);
	}

	for (count = 0, rptr = r4; rptr != NULL; count++, rptr = rptr->next)
		;

	if ((rem_r = _DtCm_alloc_reminder_references(count)) == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	i = 0;
	for (rptr = r4; rptr != NULL && stat == CSA_SUCCESS; rptr = rptr->next)
	{

		/* appts may not be in the same order as r4 so
		 * need to search for it
		 */
		for (aptr = appts; aptr != NULL; aptr = aptr->next) {
			if (rptr->appt_id.tick == aptr->appt_id.tick &&
			    rptr->appt_id.key == aptr->appt_id.key)
				break;
		}

		if (aptr == NULL)
			continue;

		(void)_csa_tick_to_iso8601(rptr->tick, isotime);
		if ((rem_r[i].run_time = strdup(isotime)) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if ((rem_r[i].attribute_name = strdup(
		    _DtCm_old_reminder_name_to_name(rptr->attr.attr)))
		    == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if ((stat = _DtCm_make_libentry(NULL, &entry))
		    == CSA_SUCCESS) {
			entry->e->key.id = rptr->appt_id.key;
			entry->e->key.time = rptr->appt_id.tick;
			rem_r[i].entry = (CSA_entry_handle)entry;
			stat = _DtCm_appt4_to_attrs(calname, aptr,
				entry->e->num_attrs, entry->e->attrs,
				_B_FALSE);
		} else
			break;

		i++;
	}

	if (stat == CSA_SUCCESS) {
		*num_rems = i;
		*rems = rem_r;
	} else {
		_DtCm_free(rem_r);
	}

	return(stat);
}

extern CSA_return_code
_DtCm_cms2csa_reminder_detail(
	Calendar			*cal,
	cms_reminder_ref		*cmsrems,
	cms_get_entry_attr_res_item	*entries,
	CSA_uint32			*num_rems,
	CSA_reminder_reference		**csarems)
{
	CSA_return_code		stat = CSA_SUCCESS;
	_DtCm_libentry		*entry;
        CSA_reminder_reference	*rem_r;
	cms_reminder_ref	*rptr;
	int			i, count;
	char			isotime[BUFSIZ];
	boolean_t		skip;
	cms_get_entry_attr_res_item *eptr;

	if (num_rems == NULL || csarems == NULL)
		return(CSA_E_INVALID_PARAMETER);

	if (cmsrems == NULL || entries == NULL) {
		*num_rems = 0;
		*csarems = NULL;
		return (CSA_SUCCESS);
	}

	for (count = 0, rptr = cmsrems; rptr != NULL; rptr = rptr->next)
		count++;

	if ((rem_r = _DtCm_alloc_reminder_references(count)) == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	/* order may not be the same so need to search for it */
	i = 0;
	for (rptr = cmsrems; rptr != NULL && stat == CSA_SUCCESS;
	     rptr = rptr->next)
	{
		skip = _B_FALSE;
		for (eptr = entries; eptr != NULL; eptr = eptr->next) {
			if (rptr->key.time == eptr->key.time &&
			    rptr->key.id == eptr->key.id) {
				if (eptr->stat != CSA_SUCCESS)
					skip = _B_TRUE;
				break;
			}
		}

		if (skip == _B_TRUE || eptr == NULL)
			continue;

		(void)_csa_tick_to_iso8601(rptr->runtime, isotime);
		if ((rem_r[i].run_time = strdup(isotime)) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if ((rem_r[i].attribute_name = strdup(rptr->reminder_name))
		    == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if ((stat = _DtCm_make_libentry(NULL, &entry)) == CSA_SUCCESS) {
			rem_r[i].entry = (CSA_entry_handle)entry;
			entry->e->key.id = rptr->key.id;
			entry->e->key.time = rptr->key.time;

			stat = _DtCmUpdateAttributes(eptr->num_attrs,
				eptr->attrs, &entry->e->num_attrs,
				&entry->e->attrs, &cal->entry_tbl, _B_FALSE,
				_B_TRUE);
		} else
			break;

		i++;
	}

	if (stat == CSA_SUCCESS) {
		*num_rems = i;
		*csarems = rem_r;
	} else {
		_DtCm_free(rem_r);
	}

	return(stat);
}

extern CSA_return_code
_DtCm_convert_appt_to_reminder_detail(
	char				*cname,
	cms_reminder_ref		*cmsrems,
	Appt_4				*appt,
	CSA_uint32			*num_rems,
	CSA_reminder_reference		**csarems)
{
	CSA_return_code		stat = CSA_SUCCESS;
	_DtCm_libentry		*entry;
        CSA_reminder_reference	*rem_r;
	cms_reminder_ref	*rptr;
	int			i, count;
	char			isotime[BUFSIZ];
	Appt_4			*aptr;

	if (num_rems == NULL || csarems == NULL)
		return(CSA_E_INVALID_PARAMETER);

	if (cmsrems == NULL || appt == NULL) {
		*num_rems = 0;
		*csarems = NULL;
		return (CSA_SUCCESS);
	}

	for (count = 0, rptr = cmsrems; rptr != NULL; rptr = rptr->next)
		count++;

	if ((rem_r = _DtCm_alloc_reminder_references(count)) == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	/* order may not be the same so need to search for it */
	i = 0;
	for (rptr = cmsrems; rptr != NULL && stat == CSA_SUCCESS;
	     rptr = rptr->next)
	{
		for (aptr = appt; aptr != NULL; aptr = aptr->next) {
			if (rptr->key.time == aptr->appt_id.tick &&
			    rptr->key.id == aptr->appt_id.key) {
				break;
			}
		}

		if (aptr == NULL)
			continue;

		(void)_csa_tick_to_iso8601(rptr->runtime, isotime);
		if ((rem_r[i].run_time = strdup(isotime)) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if ((rem_r[i].attribute_name = strdup(rptr->reminder_name))
		    == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if ((stat = _DtCm_make_libentry(NULL, &entry)) == CSA_SUCCESS) {
			rem_r[i].entry = (CSA_entry_handle)entry;
			entry->e->key.id = rptr->key.id;
			entry->e->key.time = rptr->key.time;

			stat = _DtCm_appt4_to_attrs(cname, aptr,
				entry->e->num_attrs, entry->e->attrs,
				_B_FALSE);
		} else
			break;

		i++;
	}

	if (stat == CSA_SUCCESS) {
		*num_rems = i;
		*csarems = rem_r;
	} else {
		_DtCm_free(rem_r);
	}

	return(stat);
}

/*
 * This routine convert the entry to an entry structure with
 * a header so that it can be freed with csa_free
 * Memory occupied by the orginal entry will be destroyed.
 */
extern _DtCm_libentry *
_DtCm_convert_entry_wheader(_DtCm_libentry *entry)
{
	_DtCm_libentry *pentry;

	if ((pentry = (_DtCm_libentry *)_DtCm_alloc_entry(
	    sizeof(_DtCm_libentry))) == NULL) {
		_DtCm_free_libentries(entry);
		return (NULL);
	}

	pentry->handle = (void *)pentry;
	pentry->e = entry->e;

	free(entry);

	return(pentry);
}

/*
 * this is invoked from csa_free indirectly
 * to free one entry.
 */
extern void
_DtCm_free_entry_content(CSA_uint32 dummy, _DtCm_libentry *entry)
{
	if (entry->e) _DtCm_free_cms_entry(entry->e);
	memset((void *)entry, NULL, sizeof(_DtCm_libentry));
}

/*
 * free a linked list of appointments
 * All the memory pointed to by the entry are freed,
 * except for the attribute array.
 * The entry structures are returned to the free list.
 */
extern void
_DtCm_free_libentries(_DtCm_libentry *entry)
{
	_DtCm_libentry *nptr;

	while (entry != NULL) {
		nptr = entry->next;

		if (entry->e) _DtCm_free_cms_entry(entry->e);
		memset((void *)entry, NULL, sizeof(_DtCm_libentry));

		free(entry);

		entry = nptr;
	}
}

/*
 * The entry objects pointed to by the array are freed as well.
 */
extern void
_DtCm_free_reminder_references(
	CSA_uint32 num_rems,
	CSA_reminder_reference *rems)
{
	int i;

	for (i = 0; i < num_rems; i++) {

		_DtCm_free_libentries((_DtCm_libentry *)rems[i].entry);

		if (rems[i].run_time)
			free(rems[i].run_time);

		if (rems[i].snooze_time)
			free(rems[i].snooze_time);

		if (rems[i].attribute_name)
			free(rems[i].attribute_name);

	}
}

/*
 * The entries in the array are linked together
 */
extern void
_DtCm_free_entry_handles(CSA_uint32 num_entries, CSA_entry_handle *entries)
{
	DP(("api.c: _DtCm_free_entry_handles\n"));

	_DtCm_free_libentries((_DtCm_libentry *)entries[0]);
}

/******************************************************************************
 * static functions used within in the file
 ******************************************************************************/

static CSA_return_code
_CmsentryToLibentry(
	_DtCmNameTable	**tbl,
	cms_entry	*e,
	_DtCm_libentry	**entry_r)
{
	_DtCm_libentry *entry;
	CSA_return_code stat;

	if ((stat = _DtCm_make_libentry(NULL, &entry)) != CSA_SUCCESS)
		return (stat); 

	if ((stat = _DtCmUpdateAttributes(e->num_attrs, e->attrs,
	    &entry->e->num_attrs, &entry->e->attrs, tbl, _B_FALSE,
	    _B_FALSE)) != CSA_SUCCESS) {
		_DtCm_free_libentries(entry);
	} else {
		entry->e->key = e->key;
		*entry_r = entry;
	}

	return (stat);
}


