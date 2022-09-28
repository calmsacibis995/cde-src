/* $XConsortium: updateattrs.c /main/cde1_maint/3 1995/10/31 09:51:15 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)updateattrs.c	1.11 96/11/12 Sun Microsystems, Inc."

#include <stdlib.h>
#include <string.h>
#include "updateattrs.h"
#include "cmsdata.h"
#include "nametbl.h"
#include "attr.h"


/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

extern CSA_return_code
_DtCmUpdateAttribute(
	cms_attribute	*from,
	cms_attribute	*to)
{
	CSA_return_code	stat = CSA_SUCCESS;

	if (to->name.name == NULL) {
		to->name.num = from->name.num;
		if ((to->name.name = strdup(from->name.name)) == NULL)
			return (CSA_E_INSUFFICIENT_MEMORY);
	}

	if (from->value && to->value && from->value->type != to->value->type)
		return (CSA_E_INVALID_ATTRIBUTE_VALUE);

	if (from->value) {
		switch (from->value->type) {
		case CSA_VALUE_BOOLEAN:
		case CSA_VALUE_ENUMERATED:
		case CSA_VALUE_FLAGS:
		case CSA_VALUE_SINT32:
		case CSA_VALUE_UINT32:
			stat = _DtCmUpdateSint32AttrVal(from->value,
				&to->value);
			break;

		case CSA_VALUE_STRING:
		case CSA_VALUE_CALENDAR_USER:
		case CSA_VALUE_DATE_TIME:
		case CSA_VALUE_DATE_TIME_RANGE:
		case CSA_VALUE_TIME_DURATION:
			stat = _DtCmUpdateStringAttrVal(from->value,
				&to->value);
			break;

		case CSA_VALUE_REMINDER:
			stat = _DtCmUpdateReminderAttrVal(from->value,
				&to->value);
			break;

		case CSA_VALUE_ACCESS_LIST:
			stat = _DtCmUpdateAccessListAttrVal(from->value,
				&to->value);
			break;

		case CSA_VALUE_DATE_TIME_LIST:
			stat = _DtCmUpdateDateTimeListAttrVal(from->value,
				&to->value);
			break;

		case CSA_VALUE_OPAQUE_DATA:
			stat = _DtCmUpdateOpaqueDataAttrVal(from->value,
				&to->value);
			break;

		default:
			stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
		}
	} else {
		_DtCm_free_cms_attribute_value(to->value);
		to->value = NULL;
	}

	return (stat);
}

extern CSA_return_code
_DtCmUpdateAccessListAttrVal(
	cms_attribute_value *newval,
	cms_attribute_value **attrval)
{
	cms_attribute_value *val;
	cms_access_entry *newlist = NULL;

	if (newval && newval->item.access_list_value &&
	    (newlist = _DtCm_copy_cms_access_list(
	    newval->item.access_list_value)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (*attrval == NULL) {
		if ((val = (cms_attribute_value *)malloc(
		    sizeof(cms_attribute_value))) == NULL) {
			if (newlist)
				_DtCm_free_cms_access_entry(newlist);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		val->type = CSA_VALUE_ACCESS_LIST;
	} else {
		val = *attrval;
		_DtCm_free_cms_access_entry(
			(cms_access_entry *)val->item.access_list_value);
	}

	val->item.access_list_value = newlist;

	*attrval = val;

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmUpdateSint32AttrVal(
	cms_attribute_value *newval,
	cms_attribute_value **attrval)
{
	cms_attribute_value	*val;

	if (newval) {
		if (*attrval == NULL) {
			if ((val = (cms_attribute_value *)malloc(
			    sizeof(cms_attribute_value))) == NULL) {
				return (CSA_E_INSUFFICIENT_MEMORY);
			}

			val->type = newval->type;

			*attrval = val;
		}
		(*attrval)->item.sint32_value = newval->item.sint32_value;

	} else if (*attrval) {

		free(*attrval);
		*attrval = NULL;
	}

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmUpdateStringAttrVal(
	cms_attribute_value *newval,
	cms_attribute_value **attrval)
{
	cms_attribute_value	*val;
	char *newstring = NULL;

	if (newval) {
		if (newval->item.string_value &&
		    (newstring = strdup(newval->item.string_value)) == NULL)
			return (CSA_E_INSUFFICIENT_MEMORY);

		if (*attrval == NULL) {
			if ((val = (cms_attribute_value *)malloc(
			    sizeof(cms_attribute_value))) == NULL) {
				if (newstring)
					free(newstring);
				return (CSA_E_INSUFFICIENT_MEMORY);
			}

			val->type = newval->type;
		} else {
			val = *attrval;
			if (val->item.string_value)
				free(val->item.string_value);
		}

		val->item.string_value = newstring;

		*attrval = val;

	} else if (*attrval) {

		free((*attrval)->item.string_value);
		free(*attrval);
		*attrval = NULL;
	}

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmUpdateReminderAttrVal(
	cms_attribute_value *newval,
	cms_attribute_value **attrval)
{
	CSA_return_code		stat;
	cms_attribute_value	*val;
	CSA_reminder		*rem = NULL;

	if (newval && newval->item.reminder_value) {
		if ((stat = _DtCm_copy_reminder(newval->item.reminder_value,
		    &rem)) != CSA_SUCCESS)
			return (CSA_E_INSUFFICIENT_MEMORY);

		if (*attrval == NULL) {
			if ((val = (cms_attribute_value *)malloc(
			    sizeof(cms_attribute_value))) == NULL) {
				if (rem)
					_DtCm_free_reminder(rem);
				return (CSA_E_INSUFFICIENT_MEMORY);
			}

			val->type = newval->type;
		} else {
			val = *attrval;
			if (val->item.reminder_value)
				_DtCm_free_reminder(val->item.reminder_value);
		}

		val->item.reminder_value = rem;

		*attrval = val;

	} else if (*attrval) {

		_DtCm_free_reminder((*attrval)->item.reminder_value);
		free(*attrval);
		*attrval = NULL;
	}

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmUpdateDateTimeListAttrVal(
	cms_attribute_value *newval,
	cms_attribute_value **attrval)
{
	cms_attribute_value	*val;
	CSA_date_time_list	dtlist = NULL;

	if (newval && newval->item.date_time_list_value) {
		if ((dtlist = _DtCm_copy_date_time_list(
		    newval->item.date_time_list_value)) == NULL)
			return (CSA_E_INSUFFICIENT_MEMORY);

		if (*attrval == NULL) {
			if ((val = (cms_attribute_value *)malloc(
			    sizeof(cms_attribute_value))) == NULL) {
				if (dtlist)
					_DtCm_free_date_time_list(dtlist);
				return (CSA_E_INSUFFICIENT_MEMORY);
			}

			val->type = newval->type;
		} else {
			val = *attrval;
			if (val->item.date_time_list_value)
				_DtCm_free_date_time_list(
					val->item.date_time_list_value);
		}

		val->item.date_time_list_value = dtlist;

		*attrval = val;

	} else if (*attrval) {

		_DtCm_free_date_time_list((*attrval)->item.date_time_list_value);
		free(*attrval);
		*attrval = NULL;
	}

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmUpdateOpaqueDataAttrVal(
	cms_attribute_value *newval,
	cms_attribute_value **attrval)
{
	CSA_return_code		stat;
	cms_attribute_value	*val;
	CSA_opaque_data		*opq = NULL;

	if (newval && newval->item.opaque_data_value) {
		if ((stat = _DtCm_copy_opaque_data(
		    newval->item.opaque_data_value, &opq)) != CSA_SUCCESS)
			return (CSA_E_INSUFFICIENT_MEMORY);

		if (*attrval == NULL) {
			if ((val = (cms_attribute_value *)malloc(
			    sizeof(cms_attribute_value))) == NULL) {
				if (opq) _DtCm_free_opaque_data(opq);
				return (CSA_E_INSUFFICIENT_MEMORY);
			}

			val->type = newval->type;
		} else {
			val = *attrval;
			if (val->item.opaque_data_value)
				_DtCm_free_opaque_data(
					val->item.opaque_data_value);
		}

		val->item.opaque_data_value = opq;

		*attrval = val;

	} else if (*attrval) {

		_DtCm_free_opaque_data((*attrval)->item.opaque_data_value);
		free(*attrval);
		*attrval = NULL;
	}

	return (CSA_SUCCESS);
}

