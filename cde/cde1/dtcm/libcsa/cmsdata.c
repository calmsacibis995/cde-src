/* $XConsortium: cmsdata.c /main/cde1_maint/3 1995/10/31 09:50:29 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)cmsdata.c	1.20 97/01/31 Sun Microsystems, Inc."

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "csa.h"
#include "cmsdata.h"
#include "nametbl.h"
#include "attr.h"
#include <pthread.h>

/* standard calendar attr name and entry attr name table
 * It's initialization is protected in _DtCm_init_hash()
 */
_DtCmNameTable *_DtCm_cal_name_tbl;
_DtCmNameTable *_DtCm_entry_name_tbl;

/*
 * allocate a cms_entry structure and initialized with
 * all the library defined attribute names
 */
extern cms_entry *
_DtCm_make_cms_entry(_DtCmNameTable *tbl)
{
	int	i;
	cms_entry *ptr;

	if ((ptr = (cms_entry *)calloc(1, sizeof(cms_entry))) == NULL)
		return (NULL);

	/* initialize the entry with attribute names */

	if ((ptr->attrs = (cms_attribute *)calloc(1,
	    sizeof(cms_attribute)*(tbl->size + 1))) == NULL) {
		free(ptr);
		return (NULL);
	}

	for (i = 1; i <= tbl->size; i++) {
		if ((ptr->attrs[i].name.name = strdup(tbl->names[i])) == NULL) {
			/* clean up */
			ptr->num_attrs = i - 1;
			_DtCm_free_cms_entry(ptr);
			return (NULL);
		}

		ptr->attrs[i].name.num = i;
	}

	ptr->num_attrs = tbl->size;

	return (ptr);
}

extern CSA_return_code
_DtCm_copy_cms_entry(cms_entry *e, cms_entry **e_r)
{
	cms_entry *ptr;
	CSA_return_code stat;

	if (e == NULL || e_r == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((ptr = (cms_entry *)calloc(1, sizeof(cms_entry))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if ((stat = _DtCm_copy_cms_attributes(e->num_attrs, e->attrs,
	    &ptr->num_attrs, &ptr->attrs)) != CSA_SUCCESS) {
		free(ptr);
		return (stat);
	} else {
		ptr->key = e->key;
		*e_r = ptr;
		return (CSA_SUCCESS);
	}
}

extern void
_DtCm_free_cms_entry(cms_entry *entry)
{
	if (entry == NULL)
		return;

	if (entry->num_attrs > 0) {
		_DtCm_free_cms_attributes(entry->num_attrs + 1, entry->attrs);
		free(entry->attrs);
	}

	free(entry);
}

extern void
_DtCm_free_cms_entries(cms_entry *entry)
{
	cms_entry *ptr;

	while (entry) {
		ptr = entry->next;

		_DtCm_free_cms_entry(entry);

		entry = ptr;
	}
}

extern void
_DtCm_init_hash()
{
	/* name_table_lock protect the intialization of
	 * _DtCm_cal_name_tbl and _DtCm_entry_name_tbl
	 */
	static pthread_mutex_t	name_table_lock;
	static boolean_t	done = _B_FALSE;

	/*
	 * Once intialized, they will not be modified and
	 * are accessed for read only. And since the flow
	 * guarantees that they are initialzed before being
	 * accessed, locking is not needed at read time.
	 */
	pthread_mutex_lock(&name_table_lock);

	if (done == _B_FALSE) {

		/* need to check whether table is actually created */
		_DtCm_cal_name_tbl = _DtCm_make_name_table(
					_DtCM_DEFINED_CAL_ATTR_SIZE,
					_CSA_CALENDAR_ATTRIBUTE_NAMES);
		_DtCm_entry_name_tbl = _DtCm_make_name_table(
					_DtCM_DEFINED_ENTRY_ATTR_SIZE,
					_CSA_ENTRY_ATTRIBUTE_NAMES);
		done = _B_TRUE;
	}

	pthread_mutex_unlock(&name_table_lock);
}

/*
 * attr->name.num contains the correct index for the attribute
 */
extern CSA_return_code
_DtCmGrowAttrArray(
	CSA_uint32 *num_attrs,
	cms_attribute **attrs,
	cms_attribute *attr)
{
	cms_attribute	*newptr;
	CSA_return_code	stat;
	int		index;

	index = attr->name.num;

	if ((newptr = (cms_attribute *)realloc(*attrs,
	    sizeof(cms_attribute) * (index + 1))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);
	else {
		*attrs = newptr;
		memset((void *)&(*attrs)[*num_attrs+1], NULL,
			sizeof(cms_attribute) * (index - *num_attrs));
	}

	if ((stat = _DtCm_copy_cms_attribute(&(*attrs)[index], attr, _B_TRUE))
	    == CSA_SUCCESS) {

		*num_attrs = index;
	}

	return (stat);
}

