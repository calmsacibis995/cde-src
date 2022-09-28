/* $XConsortium: cmsentry.c /main/cde1_maint/4 1995/10/31 10:28:23 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)cmsentry.c	1.13 96/11/12 Sun Microsystems, Inc."

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmsentry.h"
#include "cmsdata.h"
#include "nametbl.h"
#include "attr.h"

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
static CSA_return_code
_ExtractEntryAttrsFromEntry(CSA_uint32 srcsize, cms_attribute *srcattrs,
	CSA_uint32 *dstsize, cms_attribute **dstattrs);

/*****************************************************************************
 * extern functions
 *****************************************************************************/

/*
 * Make an entry from an array of attributes,
 * This routine assumes that the hash number is correct and will
 * use it.  Will hash the name only if the hash number is <= 0.
 */
extern CSA_return_code
_DtCmsMakeHashedEntry(
	_DtCmsCalendar	*cal,
	CSA_uint32	num,
	cms_attribute	*attrs,
	cms_entry	**entry)
{
	int		i, index;
	cms_entry	*eptr;
	CSA_return_code	stat = CSA_SUCCESS;

	if (cal == NULL || entry == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((eptr = _DtCm_make_cms_entry(cal->entry_tbl)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0; i < num && stat == CSA_SUCCESS; i++) {
		if (attrs[i].name.num > 0)
			index = attrs[i].name.num;
		else
			index = _DtCm_get_index_from_table(cal->entry_tbl,
							   attrs[i].name.name);

		if (index > 0) {
			/* check type */
			if (index > _DtCM_DEFINED_ENTRY_ATTR_SIZE &&
			    attrs[i].value &&
			    attrs[i].value->type != cal->types[index])
				stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			else
				stat = _DtCm_copy_cms_attr_val(attrs[i].value,
					&eptr->attrs[index].value);
		} else if (attrs[i].value) {
			if ((stat = _DtCmExtendNameTable(attrs[i].name.name, 0,
			    attrs[i].value->type, _DtCm_entry_name_tbl,
			    _DtCM_DEFINED_ENTRY_ATTR_SIZE,
			    CSA_ENTRY_ATTRIBUTE_NAMES, &cal->entry_tbl,
			    &cal->types)) == CSA_SUCCESS) {

				attrs[i].name.num = cal->entry_tbl->size;

				stat = _DtCmGrowAttrArray(&eptr->num_attrs,
					&eptr->attrs, &attrs[i]);
			}
		}
	}

	if (stat != CSA_SUCCESS) {
		_DtCm_free_cms_entry(eptr);
		return (stat);
	}

	*entry = eptr;
	return (CSA_SUCCESS);
}

extern void
_DtCmsFreeEntryAttrResItem(cms_get_entry_attr_res_item *elist)
{
	cms_get_entry_attr_res_item *ptr;

	while (elist) {
		ptr = elist->next;

		_DtCm_free_cms_attributes(elist->num_attrs, elist->attrs);
		free(elist);

		elist = ptr;
	}
}

extern CSA_return_code
_DtCmsGetCmsEntryForClient(cms_entry *e, cms_entry **e_r, boolean_t timeonly)
{
	cms_entry	*ptr;
	CSA_return_code	stat;
	char		*summary;
	cms_attribute_value *vptr;

	if (e == NULL || e_r == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if ((ptr = (cms_entry *)calloc(1, sizeof(cms_entry))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (timeonly == _B_TRUE) {
		/* replace the content of the summary attribute to
		 * a canned string
		 */
		vptr = e->attrs[CSA_ENTRY_ATTR_SUMMARY_I].value;
		summary = vptr->item.string_value;

		switch (e->attrs[CSA_ENTRY_ATTR_TYPE_I].value->item.\
			uint32_value) {
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

	stat = _ExtractEntryAttrsFromEntry(e->num_attrs, e->attrs,
		&ptr->num_attrs, &ptr->attrs);

	if (timeonly == _B_TRUE) {
		vptr->item.string_value = summary;
	}

	if (stat != CSA_SUCCESS) {
		free(ptr);
		return (stat);
	} else {
		ptr->key = e->key;
		*e_r = ptr;
		return (CSA_SUCCESS);
	}
}

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

static CSA_return_code
_ExtractEntryAttrsFromEntry(
	CSA_uint32	srcsize,
	cms_attribute	*srcattrs,
	CSA_uint32	*dstsize,
	cms_attribute	**dstattrs)
{
	CSA_return_code	stat = CSA_SUCCESS;
	int		i, j;
	cms_attribute	*attrs;

	if (dstsize == NULL || dstattrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	*dstsize = 0;
	*dstattrs = NULL;

	if (srcsize == 0)
		return (CSA_SUCCESS);

	if ((attrs = calloc(1, sizeof(cms_attribute) * srcsize)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0, j = 0; stat == CSA_SUCCESS && i <= srcsize; i++) {
		if (srcattrs[i].value == NULL)
			continue;

		if ((stat = _DtCm_copy_cms_attribute(&attrs[j], &srcattrs[i],
		    _B_TRUE)) == CSA_SUCCESS) {
			j++;
		}
	}

	if (stat == CSA_SUCCESS && j > 0) {
		*dstsize = j;
		*dstattrs = attrs;
	} else {
		_DtCm_free_cms_attributes(j, attrs);
		free(attrs);
	}

	return (stat);
}


