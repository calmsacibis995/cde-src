/*******************************************************************************
**
**  cmsattr.c
**
**  $XConsortium: cmsattr.c /main/cde1_maint/1 1995/07/14 23:24:49 drk $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

#pragma ident "@(#)cmsattr.c	1.4 96/11/12 Sun Microsystems, Inc."

#include <stdlib.h>
#include <string.h>
#include "csa.h"
#include "cm.h"
#include "attr.h"
#include "cmsdata.h"

extern CSA_return_code
_DtCmsUpdateSint32AttrVal(
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
_DtCmsUpdateUint32AttrVal(
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
		(*attrval)->item.uint32_value = newval->item.uint32_value;

	} else if (*attrval) {

		free(*attrval);
		*attrval = NULL;
	}

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmsUpdateStringAttrVal(
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
_DtCmsUpdateAccessListAttrVal(
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
_DtCmsUpdateReminderAttrVal(
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
_DtCmsUpdateDateTimeListAttrVal(
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
_DtCmsUpdateOpaqueDataAttrVal(
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

