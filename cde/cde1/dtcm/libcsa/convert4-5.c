/* $XConsortium: convert4-5.c /main/cde1_maint/1 1995/07/17 19:54:35 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)convert4-5.c	1.17 96/11/11 Sun Microsystems, Inc."


/*
 * Routines to convert data types used in version 4 to version 5.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "csa.h"
#include "attr.h"
#include "cmsdata.h"
#include "convert4-5.h"
#include "free.h"
#include "iso8601.h"
#include "lutil.h"

#define	V4_ATTR_NUM	13

/*
 * forward declaration of static functions
 */
static unsigned int accesstype4_to_accessright(int access);
static CSA_return_code eventtype4_to_attrs(Event_Type_4 tag, uint num_attrs,
				cms_attribute *attrs);
static CSA_return_code _RepeatInfoToRule(Appt_4 *a4,
				cms_attribute_value **attrval);

/*****************************************************************************
 * extern functions
 *****************************************************************************/

extern CSA_return_code
_DtCm_accessentry4_to_cmsaccesslist(
	Access_Entry_4 *a4,
	cms_access_entry **alist)
{
	CSA_return_code	stat = CSA_SUCCESS;
	cms_access_entry *to, *head, *prev;

	head = prev = NULL;
	while (a4 != NULL) {
		if ((to = (cms_access_entry *)calloc(1,
		    sizeof(cms_access_entry))) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if (a4->who && (to->user = strdup(a4->who)) == NULL) {
			free(to);
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		to->rights = accesstype4_to_accessright(a4->access_type);
		to->next = NULL;

		if (head == NULL)
			head = to;
		else
			prev->next = to;

		prev = to;

		a4 = a4->next;
	}

	if (stat != CSA_SUCCESS) {
		_DtCm_free_cms_access_entry(head);
		head = NULL;
	}

	*alist = head;
	return (stat);
}

extern CSA_return_code
_DtCm_appt4_to_attrs(
	char		*calname,
	Appt_4		*a4,
	uint		num_attrs,
	cms_attribute	*attrs,
	boolean_t	rerule)
{
	CSA_return_code	stat = CSA_SUCCESS;
	Attribute_4	*reminders = a4->attr;
	char		buf[BUFSIZ];
	CSA_opaque_data	opq;
	int		index, ntimes;
	char		*ptr1, *ptr2, *domain;

	if (a4 == NULL || num_attrs < _DtCM_DEFINED_ENTRY_ATTR_SIZE)
		return (CSA_E_INVALID_PARAMETER);

	/* exception info is not converted since it's not used
	 * and we don't have an attribute for it yet
	 */

	while (stat == CSA_SUCCESS && reminders != NULL) {
		if ((index =
		    _DtCm_old_reminder_name_to_index(reminders->attr)) >= 0) {
			if ((stat = _DtCmAttr4ToReminderValue(reminders,
			    &attrs[index].value)) != CSA_SUCCESS) {
				break;
			}
		}
		reminders = reminders->next;
	}

	if (a4->appt_id.key > 0) {
		ptr1 = strchr(calname, '@');
		ptr2 = (ptr1 ? strchr(ptr1, '.') : NULL);

		if (ptr1) {
			if (ptr2 == NULL)
				domain = NULL;

			sprintf(buf, "%d:%s%s%s", a4->appt_id.key, calname,
				(ptr2 ? "" : (domain ? "." : "")),
				(ptr2 ? "" : (domain ? domain : "")));
		} else {
			sprintf(buf, "%d:%s@%s", a4->appt_id.key, calname,
				_DtCmGetHostAtDomain());
		}
		opq.size = strlen(buf);
		opq.data = (unsigned char *)buf;
	}

	if (a4->period.period != single_4 &&
	    a4->ntimes == _DtCM_OLD_REPEAT_FOREVER)
		ntimes = CSA_X_DT_DT_REPEAT_FOREVER;
	else
		ntimes = a4->ntimes;

	if (stat != CSA_SUCCESS || (a4->appt_id.key > 0 &&
		(stat = _DtCm_set_opaque_attrval(&opq,
		&attrs[CSA_ENTRY_ATTR_REFERENCE_IDENTIFIER_I].value))))
			;
	else if ((stat = _csa_tick_to_iso8601(a4->appt_id.tick, buf) ?
		CSA_E_INVALID_DATE_TIME : CSA_SUCCESS) ||
		(stat = _DtCm_set_string_attrval(buf,
		&attrs[CSA_ENTRY_ATTR_START_DATE_I].value,CSA_VALUE_DATE_TIME)))
			;
	else if (stat = eventtype4_to_attrs(a4->tag->tag, num_attrs, attrs))
			;
	else if (stat = _DtCm_set_sint32_attrval(a4->tag->showtime,
		&attrs[CSA_X_DT_ENTRY_ATTR_SHOWTIME_I].value))
			;
	else if ((stat = _csa_tick_to_iso8601(a4->appt_id.tick + a4->duration,
		buf) ? CSA_E_INVALID_DATE_TIME : CSA_SUCCESS) ||
		(stat = _DtCm_set_string_attrval(buf,
		&attrs[CSA_ENTRY_ATTR_END_DATE_I].value, CSA_VALUE_DATE_TIME)))
			;
	else if (stat = _DtCm_set_string_attrval(a4->what,
		&attrs[CSA_ENTRY_ATTR_SUMMARY_I].value, CSA_VALUE_STRING))
			;
	else if (stat = _DtCm_set_user_attrval(a4->author,
		&attrs[CSA_ENTRY_ATTR_ORGANIZER_I].value))
			;
	else if (stat = _DtCm_set_uint32_attrval(_DtCm_apptstatus4_to_status(
		a4->appt_status), &attrs[CSA_ENTRY_ATTR_STATUS_I].value))
			;
	else if (stat = _DtCm_set_uint32_attrval(
		_DtCm_privacy4_to_classification(a4->privacy),
		&attrs[CSA_ENTRY_ATTR_CLASSIFICATION_I].value))
			;
	else if (rerule && (stat = _RepeatInfoToRule(a4,
		&attrs[CSA_ENTRY_ATTR_RECURRENCE_RULE_I].value)))
			;
	else if (!rerule &&
		((stat = _DtCm_set_uint32_attrval(ntimes,
		    &attrs[CSA_X_DT_ENTRY_ATTR_REPEAT_TIMES_I].value)) ||
		(stat = _DtCm_set_sint32_attrval(_DtCm_interval4_to_rtype(
		    a4->period.period),
		    &attrs[CSA_X_DT_ENTRY_ATTR_REPEAT_TYPE_I].value)) ||
		(a4->period.period == nthWeekday_4 &&
		    (stat = _DtCm_set_sint32_attrval(a4->period.nth, &attrs\
		    [CSA_X_DT_ENTRY_ATTR_REPEAT_OCCURRENCE_NUM_I].value))) ||
		(a4->period.period >= everyNthDay_4 &&
		    a4->period.period <= everyNthMonth_4 &&
		    (stat = _DtCm_set_uint32_attrval(a4->period.nth, &attrs\
		    [CSA_X_DT_ENTRY_ATTR_REPEAT_INTERVAL_I].value))) ||
		(a4->period.enddate != 0 &&
		    ((stat = _csa_tick_to_iso8601(a4->period.enddate, buf) ?
		    CSA_E_INVALID_DATE_TIME : CSA_SUCCESS) ||
		    (stat = _DtCm_set_string_attrval(buf, &attrs\
		    [CSA_X_DT_ENTRY_ATTR_SEQUENCE_END_DATE_I].value,
		    CSA_VALUE_DATE_TIME))))))
			;
	else {
		return (CSA_SUCCESS);
	}

	/* something wrong, need cleanup */
	/* free up all attribute values set so far */

	return (stat);
}

extern int
_DtCm_eventtype4_to_type(Event_Type_4 tag)
{
	switch (tag) {
	case appointment_4:
	case holiday_4:
		return (CSA_TYPE_EVENT);

	case toDo_4:
		return (CSA_TYPE_TODO);

	case reminder_4:
	case otherTag_4:
	default:
		return (CSA_X_DT_TYPE_OTHER);
	}
}

extern char *
_DtCm_eventtype4_to_subtype(Event_Type_4 tag)
{
	switch (tag) {
	case appointment_4:
		return (CSA_SUBTYPE_APPOINTMENT);
	case holiday_4:
		return (CSA_SUBTYPE_HOLIDAY);
	case toDo_4:
	case reminder_4:
	case otherTag_4:
	default:
		return ("");
	}
}

extern CSA_sint32
_DtCm_interval4_to_rtype(Interval_4 val)
{
	switch (val) {
	case single_4:
		return (CSA_X_DT_REPEAT_ONETIME);
	case weekly_4:
		return (CSA_X_DT_REPEAT_WEEKLY);
	case daily_4:
		return (CSA_X_DT_REPEAT_DAILY);
	case biweekly_4:
		return (CSA_X_DT_REPEAT_BIWEEKLY);
	case monthly_4:
		return (CSA_X_DT_REPEAT_MONTHLY_BY_DATE);
	case yearly_4:
		return (CSA_X_DT_REPEAT_YEARLY);
	case nthWeekday_4:
		return (CSA_X_DT_REPEAT_MONTHLY_BY_WEEKDAY);
	case everyNthDay_4:
		return (CSA_X_DT_REPEAT_EVERY_NDAY);
	case everyNthWeek_4:
		return (CSA_X_DT_REPEAT_EVERY_NWEEK);
	case everyNthMonth_4:
		return (CSA_X_DT_REPEAT_EVERY_NMONTH);
	case monThruFri_4:
		return (CSA_X_DT_REPEAT_MON_TO_FRI);
	case monWedFri_4:
		return (CSA_X_DT_REPEAT_MONWEDFRI);
	case tueThur_4:
		return (CSA_X_DT_REPEAT_TUETHUR);
	case daysOfWeek_4:
		return (CSA_X_DT_REPEAT_WEEKDAYCOMBO);
	case otherPeriod_4:
	default:
		return (CSA_X_DT_REPEAT_OTHER);
	}
}

extern CSA_sint32
_DtCm_apptstatus4_to_status(Appt_Status_4 stat)
{
	switch (stat) {
	case active_4:
		return (CSA_X_DT_STATUS_ACTIVE);
	case completed_4:
		return (CSA_STATUS_COMPLETED);
	case pendingAdd_4:
		return (CSA_X_DT_STATUS_ADD_PENDING);
	case pendingDelete_4:
		return (CSA_X_DT_STATUS_DELETE_PENDING);
	case committed_4:
		return (CSA_X_DT_STATUS_COMMITTED);
	case cancelled_4:
		return (CSA_X_DT_STATUS_CANCELLED);
	default:
		return (CSA_X_DT_STATUS_ACTIVE);
	}
}

extern CSA_sint32
_DtCm_privacy4_to_classification(Privacy_Level_4 p)
{
	switch (p) {
	case public_4:
		return (CSA_CLASS_PUBLIC);
	case semiprivate_4:
		return (CSA_CLASS_CONFIDENTIAL);
	case private_4:
		return (CSA_CLASS_PRIVATE);
	default:
		return (CSA_CLASS_PUBLIC);
	}
}

/******************************************************************************
 * static functions used within the file
 ******************************************************************************/

/*
 * Static functions used within the file.
 */

/*
 * DtCM_PERMIT_ACCESS is added for backward compatibility
 */
static unsigned int
accesstype4_to_accessright(int access)
{
	unsigned int	newaccess = 0;

	if (access & access_read_4)
		newaccess |= CSA_X_DT_BROWSE_ACCESS;
	if (access & access_write_4)
		newaccess |= CSA_X_DT_INSERT_ACCESS;
	if (access & access_delete_4)
		newaccess |= CSA_X_DT_DELETE_ACCESS;

	return (newaccess);
}

extern CSA_return_code
_DtCmAttr4ToReminderValue(
	Attribute_4 *remval,
	cms_attribute_value **attrval)
{
	CSA_reminder rval;
	char timestr[BUFSIZ];

	/*
	 * rval is just a place holder, values will be
	 * copied in _DtCm_set_reminder_attrval
	 */
	_csa_duration_to_iso8601(atoi(remval->value), timestr);
	rval.lead_time = timestr;

	if (remval->clientdata && *remval->clientdata != NULL) {
		rval.reminder_data.size = strlen(remval->clientdata);
		rval.reminder_data.data = (unsigned char *)remval->clientdata;
	} else {
		rval.reminder_data.size = 0;
		rval.reminder_data.data = NULL;
	}

	rval.snooze_time = NULL;
	rval.repeat_count = 0;

	return (_DtCm_set_reminder_attrval(&rval, attrval));
}

static CSA_return_code
eventtype4_to_attrs(Event_Type_4 tag, uint num_attrs, cms_attribute *attrs)
{
	CSA_return_code stat;

	switch (tag) {
	case appointment_4:
		if ((stat = _DtCm_set_uint32_attrval(CSA_TYPE_EVENT,
		    &attrs[CSA_ENTRY_ATTR_TYPE_I].value)) != CSA_SUCCESS)
			return (stat);

		return (_DtCm_set_string_attrval(CSA_SUBTYPE_APPOINTMENT,
			&attrs[CSA_ENTRY_ATTR_SUBTYPE_I].value,
			CSA_VALUE_STRING));

	case toDo_4:
		return (_DtCm_set_uint32_attrval(CSA_TYPE_TODO,
			&attrs[CSA_ENTRY_ATTR_TYPE_I].value));

	case holiday_4:
		if ((stat = _DtCm_set_uint32_attrval(CSA_TYPE_EVENT,
		    &attrs[CSA_ENTRY_ATTR_TYPE_I].value)) != CSA_SUCCESS)
			return (stat);

		return (_DtCm_set_string_attrval(CSA_SUBTYPE_HOLIDAY,
			&attrs[CSA_ENTRY_ATTR_SUBTYPE_I].value,
			CSA_VALUE_STRING));

	case reminder_4:
	case otherTag_4:
	default:
		return (_DtCm_set_uint32_attrval(CSA_X_DT_TYPE_OTHER,
			&attrs[CSA_ENTRY_ATTR_TYPE_I].value));
	}
}

static CSA_return_code
_RepeatInfoToRule(Appt_4 *a4, cms_attribute_value **attrval)
{
	char	buf[BUFSIZ], datestr[25];
	uint	duration;

	if (a4->period.period == single_4 || a4->period.period == otherPeriod_4)	{
		*attrval = NULL;
		return (CSA_SUCCESS);
	}

	if (a4->ntimes == _DtCM_OLD_REPEAT_FOREVER) {
		duration = 0;
	} else if (a4->period.period >= everyNthDay_4 &&
	    a4->period.period <= everyNthMonth_4) {
		duration = ((a4->ntimes % a4->period.nth) ? 1 : 0) +
				a4->ntimes/a4->period.nth;
	} else
		duration = a4->ntimes;

	switch (a4->period.period) {
	case daily_4:
		sprintf(buf, "D1 #%d ", duration);
		break;
	case weekly_4:
		sprintf(buf, "W1 #%d ", duration);
		break;
	case biweekly_4:
		sprintf(buf, "W2 #%d ", duration);
		break;
	case monthly_4:
		sprintf(buf, "MD1 #%d ", duration);
		break;
	case yearly_4:
		sprintf(buf, "YM1 #%d ", duration);
		break;
	case nthWeekday_4:
		sprintf(buf, "MP1 #%d ", duration);
		break;
	case everyNthDay_4:
		sprintf(buf, "D%d #%d ", a4->period.nth, duration);
		break;
	case everyNthWeek_4:
		sprintf(buf, "W%d #%d ", a4->period.nth, duration);
		break;
	case everyNthMonth_4:
		sprintf(buf, "MD%d #%d ", a4->period.nth, duration);
		break;
	case monThruFri_4:
		sprintf(buf, "W1 MO TU WE TH FR #%d ", duration);
		break;
	case monWedFri_4:
		sprintf(buf, "W1 MO WE FR #%d ", duration);
		break;
	case tueThur_4:
		sprintf(buf, "W1 TU TH #%d ", duration);
		break;
	case daysOfWeek_4:
		sprintf(buf, "W1 #%d ", duration);
		if (a4->period.nth & 0x1) strcat(buf, "SU ");
		if (a4->period.nth & 0x2) strcat(buf, "MO ");
		if (a4->period.nth & 0x4) strcat(buf, "TU ");
		if (a4->period.nth & 0x8) strcat(buf, "WE ");
		if (a4->period.nth & 0x10) strcat(buf, "TH ");
		if (a4->period.nth & 0x20) strcat(buf, "FR ");
		if (a4->period.nth & 0x40) strcat(buf, "SA ");
		break;
	}

	if (a4->period.enddate) {
		if (_csa_tick_to_iso8601(a4->period.enddate, datestr))
			return (CSA_E_INVALID_DATE_TIME);
		strcat(buf, datestr);
	}

	return (_DtCm_set_string_attrval(buf, attrval, CSA_VALUE_STRING));
}

