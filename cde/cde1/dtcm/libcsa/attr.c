/* $XConsortium: attr.c /main/cde1_maint/4 1995/10/31 09:49:41 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)attr.c	1.50 96/11/12 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "attr.h"
#include "cmsdata.h"
#include "nametbl.h"
#include "free.h"
#include "misc.h"
#include "iso8601.h"
#include "lutil.h"
#include "updateattrs.h"

/*
 * calendar attributes defined by the library
 * Note: index zero is not used
 * 	 When this table is extended, remember to extend
 *	 it's duplicate _CSA_CALENDAR_ATTRIBUTE_NAMES
 */
char *CSA_CALENDAR_ATTRIBUTE_NAMES[] = {
	NULL,
	"-//XAPIA/CSA/CALATTR//NONSGML Access List//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Calendar Name//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Calendar Owner//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Calendar Size//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Character Set//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Country//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Date Created//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Language//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Number Entries//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Product Identifier//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Time Zone//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Version//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Work Schedule//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/CALATTR//NONSGML Server Version//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/CALATTR//NONSGML Data Version//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/CALATTR//NONSGML Calendar Delimiter//EN"
};

/*
 * This is a duplicate of CSA_CALENDAR_ATTRIBUTE_NAMES which
 * is exported in csa.h
 * _CSA_CALENDAR_ATTRIBUTE_NAMES is used internally so that
 * applications linked with older version of libcsa can
 * still works
 */
char *_CSA_CALENDAR_ATTRIBUTE_NAMES[] = {
	NULL,
	"-//XAPIA/CSA/CALATTR//NONSGML Access List//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Calendar Name//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Calendar Owner//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Calendar Size//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Character Set//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Country//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Date Created//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Language//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Number Entries//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Product Identifier//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Time Zone//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Version//EN",
	"-//XAPIA/CSA/CALATTR//NONSGML Work Schedule//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/CALATTR//NONSGML Server Version//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/CALATTR//NONSGML Data Version//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/CALATTR//NONSGML Calendar Delimiter//EN"
};

/*
 * entry attributes defined by the library
 * Note: index zero is not used
 * 	 When this table is extended, remember to extend
 *	 it's duplicate _CSA_ENTRY_ATTRIBUTE_NAMES
 */
char *CSA_ENTRY_ATTRIBUTE_NAMES[] = {
	NULL,
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Attendee List//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Audio Reminder//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Classification//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Date Completed//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Date Created//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Description//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Due Date//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML End Date//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Exception Dates//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Exception Rule//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Flashing Reminder//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Last Update//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Mail Reminder//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Number Recurrences//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Organizer//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Popup Reminder//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Priority//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Recurrence Rule//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Recurring Dates//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Reference Identifier//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Sequence Number//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Sponsor//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Start Date//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Status//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Subtype//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Summary//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Time Transparency//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Type//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Show Time//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Repeat Type//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Repeat Times//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Repeat Interval//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Repeat Occurrence Number//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Sequence End Date//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Entry Delimiter//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Character Set//EN"
};

/*
 * This is a duplicate of CSA_ENTRY_ATTRIBUTE_NAMES which
 * is exported in csa.h
 * _CSA_ENTRY_ATTRIBUTE_NAMES is used internally so that
 * applications linked with older version of libcsa can
 * still works
 */
char *_CSA_ENTRY_ATTRIBUTE_NAMES[] = {
	NULL,
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Attendee List//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Audio Reminder//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Classification//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Date Completed//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Date Created//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Description//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Due Date//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML End Date//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Exception Dates//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Exception Rule//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Flashing Reminder//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Last Update//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Mail Reminder//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Number Recurrences//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Organizer//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Popup Reminder//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Priority//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Recurrence Rule//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Recurring Dates//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Reference Identifier//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Sequence Number//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Sponsor//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Start Date//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Status//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Subtype//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Summary//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Time Transparency//EN",
	"-//XAPIA/CSA/ENTRYATTR//NONSGML Type//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Show Time//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Repeat Type//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Repeat Times//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Repeat Interval//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Repeat Occurrence Number//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Sequence End Date//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Entry Delimiter//EN",
	"-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Character Set//EN"
};

/*
 * Values for entry attribute CSA_ENTRY_ATTR_SUBTYPE
 */
char *CSA_ENTRY_SUBTYPE_VALUES[] = {
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Appointment//EN",
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Class//EN",
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Holiday//EN",
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Meeting//EN",
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Miscellaneous//EN",
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Phone Call//EN",
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Sick Day//EN",
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Special Occasion//EN",
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Travel//EN",
	"-//XAPIA/CSA/SUBTYPE//NONSGML Subtype Vacation//EN",
};

/* list of calendar attributes and value type */
_DtCmAttrInfo _CSA_cal_attr_info[] =
{
	/* first element is not used */
	{ 0, -1, 0, _DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_ACCESS_LIST_I, CSA_VALUE_ACCESS_LIST,	1,
		_DtCm_old_attr_unknown, _B_FALSE, _B_FALSE },
	{ CSA_CAL_ATTR_CALENDAR_NAME_I, CSA_VALUE_STRING,	1,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_CALENDAR_OWNER_I, CSA_VALUE_CALENDAR_USER,	1,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_CALENDAR_SIZE_I,	CSA_VALUE_UINT32,	4,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_CHARACTER_SET_I,	CSA_VALUE_STRING,	4,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_COUNTRY_I,	CSA_VALUE_STRING,	0,
		_DtCm_old_attr_unknown, _B_FALSE, _B_FALSE },
	{ CSA_CAL_ATTR_DATE_CREATED_I,	CSA_VALUE_DATE_TIME,	4,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_LANGUAGE_I,	CSA_VALUE_STRING,	0,
		_DtCm_old_attr_unknown, _B_FALSE, _B_FALSE },
	{ CSA_CAL_ATTR_NUMBER_ENTRIES_I, CSA_VALUE_UINT32,	1,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_PRODUCT_IDENTIFIER_I, CSA_VALUE_STRING,	1,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_TIME_ZONE_I,	CSA_VALUE_STRING,	4,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_VERSION_I,	CSA_VALUE_STRING,	1,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_CAL_ATTR_WORK_SCHEDULE_I,	CSA_VALUE_OPAQUE_DATA,	0,
		_DtCm_old_attr_unknown, _B_FALSE, _B_FALSE },
	{ CSA_X_DT_CAL_ATTR_SERVER_VERSION_I, CSA_VALUE_UINT32,	1,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_X_DT_CAL_ATTR_DATA_VERSION_I, CSA_VALUE_UINT32,	1,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_X_DT_CAL_ATTR_CAL_DELIMITER_I, CSA_VALUE_STRING,	-1,
		_DtCm_old_attr_unknown, _B_TRUE, _B_TRUE }
};

/* list of entry attributes and value type */
_DtCmAttrInfo _CSA_entry_attr_info[] =
{
	/* first element is not used */
	{ 0, -1, 0, _DtCm_old_attr_unknown, _B_TRUE, _B_TRUE },
	{ CSA_ENTRY_ATTR_ATTENDEE_LIST_I,	CSA_VALUE_ATTENDEE_LIST,
	  0,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_AUDIO_REMINDER_I,	CSA_VALUE_REMINDER,
	  1,	_DtCm_old_attr_beep_reminder,	_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_CLASSIFICATION_I,	CSA_VALUE_UINT32,
	  2,	_DtCm_old_attr_privacy,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_DATE_COMPLETED_I,	CSA_VALUE_DATE_TIME,
	  4,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_DATE_CREATED_I,	CSA_VALUE_DATE_TIME,
	  4,	_DtCm_old_attr_unknown,		_B_TRUE, _B_TRUE },
	{ CSA_ENTRY_ATTR_DESCRIPTION_I,		CSA_VALUE_STRING,
	  4,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_DUE_DATE_I,		CSA_VALUE_DATE_TIME,
	  4,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_END_DATE_I,		CSA_VALUE_DATE_TIME,
	  1,	_DtCm_old_attr_duration,	_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_EXCEPTION_DATES_I,	CSA_VALUE_DATE_TIME_LIST,
	  4,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_EXCEPTION_RULE_I,	CSA_VALUE_STRING,
	  0,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_FLASHING_REMINDER_I,	CSA_VALUE_REMINDER,
	  1,	_DtCm_old_attr_flash_reminder,	_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_LAST_UPDATE_I,		CSA_VALUE_DATE_TIME,
	  4,	_DtCm_old_attr_unknown,		_B_TRUE, _B_TRUE },
	{ CSA_ENTRY_ATTR_MAIL_REMINDER_I,	CSA_VALUE_REMINDER,
	  1,	_DtCm_old_attr_mail_reminder,	_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_NUMBER_RECURRENCES_I,	CSA_VALUE_UINT32,
	  4,	_DtCm_old_attr_unknown,		_B_TRUE, _B_TRUE },
	{ CSA_ENTRY_ATTR_ORGANIZER_I,		CSA_VALUE_CALENDAR_USER,
	  1,	_DtCm_old_attr_author,		_B_TRUE, _B_TRUE },
	{ CSA_ENTRY_ATTR_POPUP_REMINDER_I,	CSA_VALUE_REMINDER,
	  1,	_DtCm_old_attr_popup_reminder,	_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_PRIORITY_I,		CSA_VALUE_UINT32,
	  4,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_RECURRENCE_RULE_I,	CSA_VALUE_STRING,
	  4,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_RECURRING_DATES_I,	CSA_VALUE_DATE_TIME_LIST,
	  0,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_REFERENCE_IDENTIFIER_I, CSA_VALUE_OPAQUE_DATA,	
	  1,	_DtCm_old_attr_id,		_B_TRUE, _B_TRUE },
	{ CSA_ENTRY_ATTR_SEQUENCE_NUMBER_I,	CSA_VALUE_UINT32,
	  0,	_DtCm_old_attr_unknown,		_B_TRUE, _B_TRUE },
	{ CSA_ENTRY_ATTR_SPONSOR_I,		CSA_VALUE_CALENDAR_USER,
	  4,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_START_DATE_I,		CSA_VALUE_DATE_TIME,
	  1,	_DtCm_old_attr_time,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_STATUS_I,		CSA_VALUE_UINT32,
	  2,	_DtCm_old_attr_status,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_SUBTYPE_I,		CSA_VALUE_STRING,
	  1,	_DtCm_old_attr_type2,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_SUMMARY_I,		CSA_VALUE_STRING,
	  1,	_DtCm_old_attr_what,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_TIME_TRANSPARENCY_I,	CSA_VALUE_SINT32,
	  4,	_DtCm_old_attr_unknown,		_B_FALSE, _B_FALSE },
	{ CSA_ENTRY_ATTR_TYPE_I,		CSA_VALUE_UINT32,
	  1,	_DtCm_old_attr_type,		_B_FALSE, _B_FALSE },
	{ CSA_X_DT_ENTRY_ATTR_SHOWTIME_I,	CSA_VALUE_SINT32,
	  2,	_DtCm_old_attr_showtime,	_B_FALSE, _B_FALSE },
	{ CSA_X_DT_ENTRY_ATTR_REPEAT_TYPE_I,	CSA_VALUE_SINT32,
	  1,	_DtCm_old_attr_repeat_type,	_B_FALSE, _B_TRUE },
	{ CSA_X_DT_ENTRY_ATTR_REPEAT_TIMES_I,	CSA_VALUE_UINT32,
	  1,	_DtCm_old_attr_repeat_times,	_B_FALSE, _B_TRUE },
	{ CSA_X_DT_ENTRY_ATTR_REPEAT_INTERVAL_I, CSA_VALUE_UINT32,
	  3,	_DtCm_old_attr_repeat_nth_interval, _B_FALSE, _B_TRUE },
	{ CSA_X_DT_ENTRY_ATTR_REPEAT_OCCURRENCE_NUM_I, CSA_VALUE_SINT32,
	  3,	_DtCm_old_attr_repeat_nth_weeknum, _B_FALSE, _B_TRUE },
	{ CSA_X_DT_ENTRY_ATTR_SEQUENCE_END_DATE_I, CSA_VALUE_DATE_TIME,
	  3,	_DtCm_old_attr_end_date,	_B_FALSE, _B_TRUE },
	{ CSA_X_DT_ENTRY_ATTR_ENTRY_DELIMITER_I, CSA_VALUE_STRING,
	  -1,	_DtCm_old_attr_unknown,		_B_TRUE, _B_TRUE },
	{ CSA_X_DT_ENTRY_ATTR_CHARACTER_SET_I, CSA_VALUE_STRING,
	  4,	_DtCm_old_attr_unknown,		_B_TRUE, _B_FALSE }
};

/*****************************************************************************
 * forward declaration of static functions used within the file
 *****************************************************************************/
static CSA_return_code check_predefined_attrs(
	int			fversion,
	CSA_uint32		num_attrs,
	cms_attribute		*attrs,
	boolean_t		checkreadonly,
	_DtCmNameTable		*tbl,
	CSA_uint32		num_defined,
	_DtCmAttrInfo		*our_attrs);

static CSA_return_code convert_cms_user_to_csa_user(char *from,
	CSA_calendar_user **to);

static CSA_return_code hash_entry_attrs(CSA_uint32 num_attrs,
	CSA_attribute *csaattrs, cms_attribute *cmsattrs, CSA_uint32 *hnum,
	cms_attribute **hattrs, CSA_uint32 *num);

static CSA_return_code hash_cal_attrs(CSA_uint32 num_attrs,
	CSA_attribute *csaattrs, cms_attribute *cmsattrs, CSA_uint32 *hnum,
	cms_attribute **hattrs, CSA_uint32 *num);

static CSA_return_code _DtCm_check_hashed_entry_attributes(
	int		fvers,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	CSA_flags	utype);

static CSA_return_code _DtCm_check_hashed_cal_attributes(
	int		fvers,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	char		*owner,
	char		*cname,
	boolean_t	checkreadonly,
	boolean_t	firsttime,
	boolean_t	csatype);

static CSA_return_code _CheckNameAtHost(char *owner, char *value,
					boolean_t strict);

static CSA_return_code _CheckCalendarOwner(char *owner, char *cname, int type,
					char *name);

static CSA_return_code _CheckCalendarName(char *owner, char *cname,
					cms_attribute_value *val);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

/*
 * For each calendar attribute, if it is a predefined attribute,
 * check that the data type is correct.
 * If checkreadonly is _B_TRUE, also check that it's not readonly.
 */
extern CSA_return_code
_DtCm_check_cal_csa_attributes(
	int		fvers,
	CSA_uint32	num_attrs,
	CSA_attribute	*attrs,
	char		*cname,
	boolean_t	checkreadonly,
	boolean_t	firsttime,
	boolean_t	checkattrnum)
{
	CSA_return_code stat;
	CSA_uint32	hnum;
	cms_attribute	*hattrs;
	CSA_uint32	realnum;

	if ((stat = hash_cal_attrs(num_attrs, attrs, NULL, &hnum, &hattrs,
	    &realnum)) != CSA_SUCCESS)
		return (stat);

	if (checkattrnum == _B_TRUE && realnum == 0)
		return (CSA_E_INVALID_PARAMETER);

	stat = _DtCm_check_hashed_cal_attributes(fvers, hnum, hattrs,
		NULL, cname, checkreadonly, firsttime, _B_TRUE);

	free(hattrs);

	return (stat);
}

extern CSA_return_code
_DtCm_check_cal_cms_attributes(
	int		fvers,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	char		*owner,
	char		*cname,
	boolean_t	checkreadonly,
	boolean_t	firsttime,
	boolean_t	checkattrnum)
{
	CSA_return_code stat;
	CSA_uint32	hnum;
	cms_attribute	*hattrs;
	CSA_uint32	realnum;

	if ((stat = hash_cal_attrs(num_attrs, NULL, attrs, &hnum, &hattrs,
	    &realnum)) != CSA_SUCCESS)
		return (stat);

	if (checkattrnum == _B_TRUE && realnum == 0)
		return (CSA_E_INVALID_PARAMETER);

	stat = _DtCm_check_hashed_cal_attributes(fvers, hnum, hattrs,
		owner, cname, checkreadonly, firsttime, _B_FALSE);

	free(hattrs);

	return (stat);
}

extern CSA_return_code
_DtCm_check_entry_attributes(
	int		fvers,
	CSA_uint32	num_attrs,
	CSA_attribute	*attrs,
	CSA_flags	utype,
	boolean_t	checkattrnum)
{
	CSA_return_code	stat;
	CSA_uint32	hnum;
	cms_attribute	*hattrs;
	CSA_uint32	realnum;

	if ((stat = hash_entry_attrs(num_attrs, attrs, NULL, &hnum, &hattrs,
	    &realnum)) != CSA_SUCCESS)
		return (stat);

	if (checkattrnum == _B_TRUE && realnum == 0)
		return (CSA_E_INVALID_PARAMETER);

	stat = _DtCm_check_hashed_entry_attributes(fvers, hnum, hattrs,
		utype);

	free(hattrs);

	return (stat);
}

extern CSA_return_code
_DtCm_check_entry_cms_attributes(
	int		fvers,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	CSA_flags	utype,
	boolean_t	checkattrnum)
{
	CSA_return_code	stat;
	CSA_uint32	hnum;
	cms_attribute	*hattrs;
	CSA_uint32	realnum;

	if ((stat = hash_entry_attrs(num_attrs, NULL, attrs, &hnum, &hattrs,
	    &realnum)) != CSA_SUCCESS)
		return (stat);

	if (checkattrnum == _B_TRUE && realnum == 0)
		return (CSA_E_INVALID_PARAMETER);

	stat = _DtCm_check_hashed_entry_attributes(fvers, hnum, hattrs,
		utype);

	free(hattrs);

	return (stat);
}

/*
 * copy attributes
 * attributes with a name but NULL value is allowed
 * attributes with null names are ignored
 * validity of attributes should be checked before calling this routine
 *
 * Note: the first entry is not used
 */
extern CSA_return_code
_DtCm_copy_cms_attributes(
	CSA_uint32 srcsize,
	cms_attribute *srcattrs,
	CSA_uint32 *dstsize,
	cms_attribute **dstattrs)
{
	int	i, j;
	CSA_return_code stat = CSA_SUCCESS;
	cms_attribute	*attrs;

	if (dstsize == NULL || dstattrs == NULL)
		return (CSA_E_INVALID_PARAMETER);

	*dstsize = 0;
	*dstattrs = NULL;

	if (srcsize == 0)
		return (CSA_SUCCESS);

	if ((attrs = calloc(1, sizeof(cms_attribute) * (srcsize + 1))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* firstr element is not used */
	for (i = 1, j = 1; i <= srcsize; i++) {
		if (srcattrs[i].name.name != NULL) {
			if ((stat = _DtCm_copy_cms_attribute(&attrs[j],
			    &srcattrs[i], _B_TRUE)) != CSA_SUCCESS)
				break;
			else
				j++;
		}
	}

	if (stat != CSA_SUCCESS && j > 1) {
		_DtCm_free_cms_attributes(j, attrs);
		free(attrs);
	} else {
		*dstsize = j - 1;
		*dstattrs = attrs;
	}

	return(stat);
}

/*
 * Frees the name and value field of the array, but not
 * array itself.
 * note: element 0 is not used
 */
extern void
_DtCm_free_cms_attributes(CSA_uint32 size, cms_attribute *attrs)
{
	int i;

	for (i = 0; i < size; i++) {
		if (attrs[i].name.name) {
			free(attrs[i].name.name);
			attrs[i].name.name = NULL;

			if (attrs[i].value) {
				_DtCm_free_cms_attribute_value(attrs[i].value);
				attrs[i].value = NULL;
			}
		}
	}
}

/*
 * Frees the name and value field of the array, but not
 * array itself.
 */
extern void
_DtCm_free_attributes(CSA_uint32 size, CSA_attribute * attrs)
{
	int i;

	for (i = 0; i < size; i++) {
		if (attrs[i].name) {
			free(attrs[i].name);
			attrs[i].name = NULL;

			if (attrs[i].value) {
				_DtCm_free_attribute_value(attrs[i].value);
				attrs[i].value = NULL;
			}
		}
	}
}

/*
 * Free the value part of the cms_attribute structure.
 * note: element 0 is not used
 */
extern void
_DtCm_free_cms_attribute_values(CSA_uint32 size, cms_attribute *attrs)
{
	int	i;

	for (i = 1; i <= size; i++) {
		if (attrs[i].value) {
			_DtCm_free_cms_attribute_value(attrs[i].value);
			attrs[i].value = NULL;
		}
	}
}

/*
 * Free the value part of the the attribute structure.
 */
extern void
_DtCm_free_attribute_values(CSA_uint32 size, CSA_attribute * attrs)
{
	int	i;

	for (i = 0; i < size; i++) {
		if (attrs[i].value) {
			_DtCm_free_attribute_value(attrs[i].value);
			attrs[i].value = NULL;
		}
	}
}

extern char *
_DtCm_old_reminder_name_to_name(char *oldname)
{
	if (strcmp(oldname, _DtCM_OLD_ATTR_BEEP_REMINDER) == 0)
		return (CSA_ENTRY_ATTR_AUDIO_REMINDER);
	else if (strcmp(oldname, _DtCM_OLD_ATTR_FLASH_REMINDER) == 0)
		return (CSA_ENTRY_ATTR_FLASHING_REMINDER);
	else if (strcmp(oldname, _DtCM_OLD_ATTR_MAIL_REMINDER) == 0)
		return (CSA_ENTRY_ATTR_MAIL_REMINDER);
	else if (strcmp(oldname, _DtCM_OLD_ATTR_POPUP_REMINDER) == 0)
		return (CSA_ENTRY_ATTR_POPUP_REMINDER);
	else
		return (oldname);
}

extern int
_DtCm_old_reminder_name_to_index(char *oldname)
{
	char	*name;

	name = _DtCm_old_reminder_name_to_name(oldname);

	return (_DtCm_get_index_from_table(_DtCm_entry_name_tbl, name));
}

/*
 * Given an attribute name, return the corresponding
 * attribute number that's supported by old backends (v4 and before).
 */
extern CSA_return_code
_DtCm_get_old_attr_by_name(char *name, _DtCm_old_attrs *attr)
{
	int	index;

	index = _DtCm_get_index_from_table(_DtCm_entry_name_tbl, name);
	if (index > 0 && index <= _DtCM_DEFINED_ENTRY_ATTR_SIZE) {

		if (_CSA_entry_attr_info[index].oldattr
		    != _DtCm_old_attr_unknown) {
			*attr = _CSA_entry_attr_info[index].oldattr;
			return (CSA_SUCCESS);
		} else
			return (CSA_E_UNSUPPORTED_ATTRIBUTE);
	} else
		return (CSA_E_INVALID_ATTRIBUTE);
}

/*
 * Given an attribute index, return the corresponding
 * attribute number that's supported by old backends (v4 and before).
 */
extern CSA_return_code
_DtCm_get_old_attr_by_index(int index, _DtCm_old_attrs *attr)
{
	if (index <= _DtCM_DEFINED_ENTRY_ATTR_SIZE) {
		if (_CSA_entry_attr_info[index].oldattr
		    != _DtCm_old_attr_unknown) {
			*attr = _CSA_entry_attr_info[index].oldattr;
			return (CSA_SUCCESS);
		} else
			return (CSA_E_UNSUPPORTED_ATTRIBUTE);
	} else
		return (CSA_E_INVALID_ATTRIBUTE);
}

/*
 * copy attribute
 * the attribute structure should contain valid value
 * a NULL attribute value is valid
 */
extern CSA_return_code
_DtCm_copy_cms_attribute(
	cms_attribute *to,
	cms_attribute *from,
	boolean_t copyname)
{
	CSA_return_code	stat = CSA_SUCCESS;
	char		*name;

	if (to == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* copy the attribute name */
	if (copyname) {
		if ((name = strdup(from->name.name)) == NULL)
			return(CSA_E_INSUFFICIENT_MEMORY);
	}

	if ((stat = _DtCm_copy_cms_attr_val(from->value, &to->value))
	    == CSA_SUCCESS) {
		if (copyname) {
			to->name.name = name;
			to->name.num = from->name.num;
		}
	} else if (copyname)
		free (name);

	return(stat);
}

extern CSA_return_code
_DtCm_copy_cms_attr_val(cms_attribute_value *from, cms_attribute_value **to)
{
	CSA_return_code	stat = CSA_SUCCESS;
	cms_attribute_value	*val;

	if (to == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* copy the attribute value */
	if (from == NULL)
		val = NULL;
	else {
		if ((val = (cms_attribute_value *)calloc(1,
		    sizeof(cms_attribute_value))) == NULL)
			return (CSA_E_INSUFFICIENT_MEMORY);

		switch (from->type) {
		case CSA_VALUE_BOOLEAN:
		case CSA_VALUE_ENUMERATED:
		case CSA_VALUE_FLAGS:
		case CSA_VALUE_UINT32:
		case CSA_VALUE_SINT32:
			val->item.uint32_value = from->item.uint32_value;
			break;

		case CSA_VALUE_STRING:
		case CSA_VALUE_DATE_TIME:
		case CSA_VALUE_DATE_TIME_RANGE:
		case CSA_VALUE_TIME_DURATION:
		case CSA_VALUE_CALENDAR_USER:
			if (from->item.string_value)
				val->item.string_value =
					strdup(from->item.string_value);
			else
				val->item.string_value = calloc(1, 1);
			if (val->item.string_value == NULL)
				stat = CSA_E_INSUFFICIENT_MEMORY;
			break;

		case CSA_VALUE_REMINDER:
			if (from->item.reminder_value == NULL)
				stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			else
				stat = _DtCm_copy_reminder(
					from->item.reminder_value,
					&val->item.reminder_value);
			break;
		case CSA_VALUE_ATTENDEE_LIST:
			stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			break;
		case CSA_VALUE_ACCESS_LIST:
			if (from->item.access_list_value &&
			    (val->item.access_list_value =
			    _DtCm_copy_cms_access_list(
			    from->item.access_list_value)) == NULL) {

				stat = CSA_E_INSUFFICIENT_MEMORY;
			}
			break;
		case CSA_VALUE_DATE_TIME_LIST:
			if (from->item.date_time_list_value &&
			    (val->item.date_time_list_value =
			    _DtCm_copy_date_time_list(
		    	    from->item.date_time_list_value)) == NULL) {

				stat = CSA_E_INSUFFICIENT_MEMORY;
			}
			break;
		case CSA_VALUE_OPAQUE_DATA:
			if (from->item.opaque_data_value) {
				stat = _DtCm_copy_opaque_data(
					from->item.opaque_data_value,
					&val->item.opaque_data_value);
			}
			break;
		default:
			stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			break;
		}

		if (stat != CSA_SUCCESS)
			free(val);
		else
			val->type = from->type;
	}

	if (stat == CSA_SUCCESS) {
		*to = val;
	}

	return(stat);
}

/*
 * copy the attribute name, and convert the attribute value
 */
extern CSA_return_code
_DtCm_cms2csa_attribute(cms_attribute from, CSA_attribute *to)
{
	CSA_return_code		stat;
	char			*name;
	CSA_attribute_value	*val;

	if ((name = strdup(from.name.name)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if ((stat = _DtCm_cms2csa_attrval(from.value, &val)) == CSA_SUCCESS) {
		to->name = name;
		to->value = val;
	} else
		free(name);

	return (stat);
}

extern CSA_return_code
_DtCm_cms2csa_attrval(cms_attribute_value *from, CSA_attribute_value **to)
{
	CSA_return_code	stat = CSA_SUCCESS;
	CSA_attribute_value	*val;

	if (to == NULL)
		return (CSA_E_INVALID_PARAMETER);

	/* copy the attribute value */
	if (from == NULL)
		val = NULL;
	else {
		if ((val = (CSA_attribute_value *)calloc(1,
		    sizeof(CSA_attribute_value))) == NULL)
			return (CSA_E_INSUFFICIENT_MEMORY);

		switch (from->type) {
		case CSA_VALUE_BOOLEAN:
		case CSA_VALUE_ENUMERATED:
		case CSA_VALUE_FLAGS:
		case CSA_VALUE_UINT32:
		case CSA_VALUE_SINT32:
			val->item.uint32_value = from->item.uint32_value;
			break;

		case CSA_VALUE_STRING:
		case CSA_VALUE_DATE_TIME:
		case CSA_VALUE_DATE_TIME_RANGE:
		case CSA_VALUE_TIME_DURATION:
			if (from->item.string_value)
				val->item.string_value =
					strdup(from->item.string_value);
			else
				val->item.string_value = calloc(1, 1);
			if (val->item.string_value == NULL)
				stat = CSA_E_INSUFFICIENT_MEMORY;
			break;

		case CSA_VALUE_REMINDER:
			if (from->item.reminder_value == NULL)
				stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			else
				stat = _DtCm_copy_reminder(
					from->item.reminder_value,
					&val->item.reminder_value);
			break;
		case CSA_VALUE_CALENDAR_USER:
			if (from->item.calendar_user_value == NULL)
				stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			else
				stat = convert_cms_user_to_csa_user(
					from->item.calendar_user_value,
					&val->item.calendar_user_value);
			break;
		case CSA_VALUE_ACCESS_LIST:
			stat = _DtCm_cms2csa_access_list(
				from->item.access_list_value,
				&val->item.access_list_value);
			break;
		case CSA_VALUE_ATTENDEE_LIST:
			stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			break;
		case CSA_VALUE_DATE_TIME_LIST:
			if (from->item.date_time_list_value &&
			    (val->item.date_time_list_value =
			    _DtCm_copy_date_time_list(
			    from->item.date_time_list_value)) == NULL) {
				stat = CSA_E_INSUFFICIENT_MEMORY;
			}
			break;
		case CSA_VALUE_OPAQUE_DATA:
			if (from->item.opaque_data_value) {
				stat = _DtCm_copy_opaque_data(
					from->item.opaque_data_value,
					&val->item.opaque_data_value);
			} else
				val->item.opaque_data_value = NULL;
			break;
		default:
			stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			break;
		}

		if (stat != CSA_SUCCESS)
			free(val);
		else
			val->type = from->type;
	}

	if (stat == CSA_SUCCESS) {
		*to = val;
	}

	return(stat);
}

extern void
_DtCm_free_cms_attribute_value(cms_attribute_value *val)
{
	if (val == NULL)
		return;

	switch (val->type) {
	case CSA_VALUE_STRING:
	case CSA_VALUE_DATE_TIME:
	case CSA_VALUE_DATE_TIME_RANGE:
	case CSA_VALUE_TIME_DURATION:
	case CSA_VALUE_CALENDAR_USER:
		if (val->item.string_value)
			free(val->item.string_value);
		break;

	case CSA_VALUE_REMINDER:
		if (val->item.reminder_value)
			_DtCm_free_reminder(val->item.reminder_value);
		break;
	case CSA_VALUE_ACCESS_LIST:
		if (val->item.access_list_value)
			_DtCm_free_cms_access_entry(val->item.access_list_value);
		break;
	case CSA_VALUE_DATE_TIME_LIST:
		if (val->item.date_time_list_value)
			_DtCm_free_date_time_list(
				val->item.date_time_list_value);
		break;
	case CSA_VALUE_OPAQUE_DATA:
		if (val->item.opaque_data_value) {
			_DtCm_free_opaque_data(val->item.opaque_data_value);
		}
		break;
	}
	free(val);
}

extern void
_DtCm_free_attribute_value(CSA_attribute_value *val)
{
	if (val == NULL)
		return;

	switch (val->type) {
	case CSA_VALUE_STRING:
	case CSA_VALUE_DATE_TIME:
	case CSA_VALUE_DATE_TIME_RANGE:
	case CSA_VALUE_TIME_DURATION:
		if (val->item.string_value)
			free(val->item.string_value);
		break;

	case CSA_VALUE_REMINDER:
		if (val->item.reminder_value)
			_DtCm_free_reminder(val->item.reminder_value);
		break;
	case CSA_VALUE_ACCESS_LIST:
		if (val->item.access_list_value)
			_DtCm_free_csa_access_list(val->item.access_list_value);
		break;
	case CSA_VALUE_CALENDAR_USER:
		if (val->item.calendar_user_value) {
			if (val->item.calendar_user_value->user_name)
				free(val->item.calendar_user_value->user_name);
			if (val->item.calendar_user_value->calendar_address)
				free(val->item.calendar_user_value->user_name);
			free(val->item.calendar_user_value);
		}
		break;
	case CSA_VALUE_DATE_TIME_LIST:
		if (val->item.date_time_list_value)
			_DtCm_free_date_time_list(
				val->item.date_time_list_value);
		break;
	case CSA_VALUE_OPAQUE_DATA:
		if (val->item.opaque_data_value) {
			_DtCm_free_opaque_data(val->item.opaque_data_value);
		}
		break;
	}
	free(val);
}

extern CSA_return_code
_DtCm_set_uint32_attrval(CSA_uint32 numval, cms_attribute_value **attrval)
{
	cms_attribute_value	*val;

	if ((val = (cms_attribute_value *)malloc(sizeof(cms_attribute_value)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	val->type = CSA_VALUE_UINT32;
	val->item.uint32_value = numval;

	*attrval = val;

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCm_set_sint32_attrval(CSA_sint32 numval, cms_attribute_value **attrval)
{
	cms_attribute_value	*val;

	if ((val = (cms_attribute_value *)malloc(sizeof(cms_attribute_value)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	val->type = CSA_VALUE_SINT32;
	val->item.sint32_value = numval;

	*attrval = val;

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCm_set_string_attrval(
	char *strval,
	cms_attribute_value **attrval,
	CSA_enum type)
{
	cms_attribute_value	*val;

	if (type != CSA_VALUE_STRING && type != CSA_VALUE_DATE_TIME &&
	    type != CSA_VALUE_DATE_TIME_RANGE &&
	    type != CSA_VALUE_TIME_DURATION && type != CSA_VALUE_CALENDAR_USER)
		return (CSA_E_INVALID_PARAMETER);

	if ((val = (cms_attribute_value *)malloc(sizeof(cms_attribute_value)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	val->type = type;

	if (strval == NULL) {
		val->item.string_value = NULL;
	} else if ((val->item.string_value = strdup(strval)) == NULL) {
		free(val);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	*attrval = val;

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCm_set_user_attrval(
	char *user,
	cms_attribute_value **attrval)
{
	cms_attribute_value	*val;

	if (user == NULL) {
		*attrval = NULL;
		return (CSA_SUCCESS);
	}

	if ((val = (cms_attribute_value *)malloc(sizeof(cms_attribute_value)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	val->type = CSA_VALUE_CALENDAR_USER;

	if ((val->item.calendar_user_value = strdup(user)) == NULL) {
		free(val);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	*attrval = val;

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCm_set_reminder_attrval(CSA_reminder *remval, cms_attribute_value **attrval)
{
	cms_attribute_value *val;
	CSA_return_code stat;

	if (remval == NULL)
		return (CSA_E_INVALID_ATTRIBUTE_VALUE);

	if ((val = (cms_attribute_value *)malloc(sizeof(cms_attribute_value)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	val->type = CSA_VALUE_REMINDER;

	if ((stat = _DtCm_copy_reminder(remval, &val->item.reminder_value))
	    != CSA_SUCCESS) {
		free(val);
		return (stat);
	} else {

		*attrval = val;

		return (CSA_SUCCESS);
	}
}

extern CSA_return_code
_DtCm_set_csa_access_attrval(
	cms_access_entry *aval,
	CSA_attribute_value **attrval)
{
	CSA_attribute_value *val;
	CSA_return_code stat = CSA_SUCCESS;

	if ((val = (CSA_attribute_value *)malloc(sizeof(CSA_attribute_value)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	val->type = CSA_VALUE_ACCESS_LIST;

	if (aval == NULL) {

		val->item.access_list_value = NULL;

	} else {
	
		stat = _DtCm_cms2csa_access_list(aval,
			&val->item.access_list_value);

	}

	if (stat == CSA_SUCCESS)
		*attrval = val;
	else
		free(val);

	return (stat);
}

extern CSA_return_code
_DtCm_set_opaque_attrval(CSA_opaque_data *data, cms_attribute_value **attrval)
{
	CSA_return_code stat;
	cms_attribute_value	*val;

	if (data == NULL)
		return (CSA_E_INVALID_ATTRIBUTE_VALUE);

	if ((val = (cms_attribute_value *)malloc(sizeof(cms_attribute_value)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	val->type = CSA_VALUE_OPAQUE_DATA;

	if ((stat = _DtCm_copy_opaque_data(data, &val->item.opaque_data_value))
	    != CSA_SUCCESS) {
		free(val);
		return (stat);
	} else {

		*attrval = val;
		return (CSA_SUCCESS);
	}
}

extern CSA_return_code
_DtCm_set_csa_uint32_attrval(CSA_uint32 numval, CSA_attribute_value **attrval)
{
	CSA_attribute_value	*val;

	if ((val = (CSA_attribute_value *)malloc(sizeof(CSA_attribute_value)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	val->type = CSA_VALUE_UINT32;
	val->item.uint32_value = numval;

	*attrval = val;

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCm_set_csa_string_attrval(
	char *strval,
	CSA_attribute_value **attrval,
	CSA_enum type)
{
	CSA_attribute_value	*val;

	if (type != CSA_VALUE_STRING && type != CSA_VALUE_DATE_TIME &&
	    type != CSA_VALUE_DATE_TIME_RANGE &&
	    type != CSA_VALUE_TIME_DURATION && type != CSA_VALUE_CALENDAR_USER)
		return (CSA_E_INVALID_PARAMETER);

	if ((val = (CSA_attribute_value *)malloc(sizeof(CSA_attribute_value)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	val->type = type;

	if (strval == NULL) {
		val->item.string_value = NULL;
	} else if ((val->item.string_value = strdup(strval)) == NULL) {
		free(val);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	*attrval = val;

	return (CSA_SUCCESS);
}

extern void
_DtCm_free_csa_access_list(CSA_access_list alist)
{
	CSA_access_list nptr;

	while (alist != NULL) {
		nptr = alist->next;

		if (alist->user) {
			if (alist->user->user_name)
				free(alist->user->user_name);
			if (alist->user->calendar_address)
				free(alist->user->calendar_address);
			free(alist->user);
		}

		free(alist);

		alist = nptr;
	}
}

extern void
_DtCm_free_date_time_list(CSA_date_time_list list)
{
	CSA_date_time_entry *nptr;

	while (list != NULL) {
		nptr = list->next;

		if (list->date_time) {
			free(list->date_time);
		}

		free(list);

		list = nptr;
	}
}

extern void
_DtCm_free_cms_access_entry(cms_access_entry *list)
{
	cms_access_entry *nptr;

	while (list != NULL) {
		nptr = list->next;

		if (list->user) {
			free(list->user);
		}

		free(list);

		list = nptr;
	}
}

extern cms_access_entry *
_DtCm_copy_cms_access_list(cms_access_entry *alist)
{
	cms_access_entry	*l, *head, *prev;
	boolean_t	cleanup = _B_FALSE;

	prev = head = NULL;
	while (alist != NULL) {
		if ((l = (cms_access_entry *)calloc(1, sizeof(cms_access_entry)))
		    == NULL) {
			cleanup = _B_TRUE;
			break;
		}

		if ((l->user = strdup(alist->user)) == NULL) {
			free(l);
			cleanup = _B_TRUE;
			break;
		}

		l->rights = alist->rights;
		l->next = NULL;

		if (head == NULL)
			head = l;
		else
			prev->next = l;
		prev = l;

		alist = alist->next;
	}

	if (cleanup == _B_TRUE) {
		_DtCm_free_cms_access_entry(head);
		head = NULL;
	}
	return(head);
}

extern CSA_return_code
_DtCm_cms2csa_access_list(
	cms_access_entry *cmslist,
	CSA_access_rights **csalist)
{
	CSA_return_code stat = CSA_SUCCESS;
	CSA_access_rights *to, *head, *prev;

	head = prev = NULL;
	while (cmslist != NULL) {
		if ((to = (CSA_access_rights *)calloc(1,
		    sizeof(CSA_access_rights))) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if ((to->user = (CSA_calendar_user *)calloc(1,
		    sizeof(CSA_calendar_user))) == NULL) {
			free(to);
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if ((to->user->user_name = strdup(cmslist->user)) == NULL) {
			free(to->user);
			free(to);
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		to->rights = cmslist->rights;
		to->next = NULL;

		if (head == NULL)
			head = to;
		else
			prev->next = to;

		prev = to;

		cmslist = cmslist->next;
	}

	if (stat != CSA_SUCCESS) {
		_DtCm_free_csa_access_list(head);
		head = NULL;
	}

	*csalist = head;
	return (stat);
}

extern CSA_return_code
_DtCm_csa2cms_access_list(
	CSA_access_rights *csalist,
	cms_access_entry **cmslist)
{
	CSA_return_code stat = CSA_SUCCESS;
	cms_access_entry *to, *head, *prev;

	head = prev = NULL;
	while (csalist != NULL) {
		if ((to = (cms_access_entry *)calloc(1,
		    sizeof(cms_access_entry))) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if (csalist->user->user_name) {
			if ((to->user = strdup(csalist->user->user_name))
			    == NULL) {
				stat = CSA_E_INSUFFICIENT_MEMORY;
				free(to);
				break;
			}
		} else {
			stat = CSA_E_INVALID_ATTRIBUTE_VALUE;
			free(to);
			break;
		}

		to->rights = csalist->rights;
		to->next = NULL;

		if (head == NULL)
			head = to;
		else
			prev->next = to;

		prev = to;

		csalist = csalist->next;
	}

	if (stat != CSA_SUCCESS) {
		_DtCm_free_cms_access_entry(head);
		head = NULL;
	}

	*cmslist = head;
	return (stat);
}

extern CSA_date_time_list
_DtCm_copy_date_time_list(CSA_date_time_list dlist)
{
	CSA_date_time_entry	*l, *head, *prev;
	boolean_t	cleanup = _B_FALSE;

	prev = head = NULL;
	while (dlist != NULL) {
		if ((l = (CSA_date_time_entry *)calloc(1,
		    sizeof(CSA_date_time_entry))) == NULL) {
			cleanup = _B_TRUE;
			break;
		}

		if ((l->date_time = strdup(dlist->date_time)) == NULL) {
			free(l);
			cleanup = _B_TRUE;
			break;
		}

		l->next = NULL;

		if (head == NULL)
			head = l;
		else
			prev->next = l;
		prev = l;

		dlist = dlist->next;
	}

	if (cleanup == _B_TRUE) {
		_DtCm_free_date_time_list(head);
		head = NULL;
	}
	return(head);
}

extern CSA_return_code
_DtCm_copy_reminder(CSA_reminder *from, CSA_reminder **to)
{
	CSA_reminder *newval;

	if ((newval = (CSA_reminder *)calloc(1, sizeof(CSA_reminder))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (from->lead_time) {
		if ((newval->lead_time = strdup(from->lead_time)) == NULL) {
			free(newval);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
	} else {
		free(newval);
		return (CSA_E_INVALID_ATTRIBUTE_VALUE);
	}

	if (from->snooze_time)
		newval->snooze_time = strdup(from->snooze_time);
	else
		newval->snooze_time = calloc(1, 1);

	if (newval->snooze_time == NULL) {
		_DtCm_free_reminder(newval);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	newval->repeat_count = from->repeat_count;

	if (from->reminder_data.size > 0) {
		newval->reminder_data.size = from->reminder_data.size;
		if ((newval->reminder_data.data = malloc(
		    newval->reminder_data.size)) == NULL) {
			_DtCm_free_reminder(newval);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		memcpy(newval->reminder_data.data, from->reminder_data.data,
			from->reminder_data.size);
	}

	*to = newval;
	return (CSA_SUCCESS);
}

extern void
_DtCm_free_reminder(CSA_reminder *val)
{
	if (val == NULL) return;

	if (val->lead_time)
		free(val->lead_time);

	if (val->snooze_time)
		free(val->snooze_time);

	if (val->reminder_data.size > 0)
		free(val->reminder_data.data);

	free(val);
}

extern CSA_return_code
_DtCm_copy_opaque_data(CSA_opaque_data *from, CSA_opaque_data **to)
{
	CSA_opaque_data	*val;

	if ((val = (CSA_opaque_data *)calloc(1, sizeof(CSA_opaque_data)))
	    == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	if (from->size > 0) {
		val->size = from->size;
		if ((val->data = malloc(from->size)) == NULL) {
			free(val);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		memcpy(val->data, from->data, from->size);
	} else {
		val->size = 0;
		val->data = NULL;
	}

	*to = val;

	return (CSA_SUCCESS);
}

extern void
_DtCm_free_opaque_data(CSA_opaque_data *val)
{
	if (val == NULL) return;

	if (val->data)
		free(val->data);
	free(val);
}

extern void
_DtCm_get_attribute_types(CSA_uint32 size, int *types)
{
	int	i;

	for (i = 1; i <= size; i++) {
		types[i] = _CSA_entry_attr_info[i].type;
	}
}

/*
 * This is called when attributes are received from
 * the server.
 */
extern CSA_return_code
_DtCmUpdateAttributes(
	CSA_uint32	numsrc,
	cms_attribute	*srcattrs,
	CSA_uint32	*numdst,
	cms_attribute	**dstattrs,
	_DtCmNameTable	**tbl,
	boolean_t	caltbl,
	boolean_t	makecopy)
{
	CSA_return_code		stat = CSA_SUCCESS;
	int			i;
	CSA_uint32		oldnum = 0;
	cms_attribute		*oldattrs;
	int			checknum;

	/* copy original attributes for rollback if update fails */
	if (makecopy && *numdst > 0 && (stat = _DtCm_copy_cms_attributes(
	    *numdst, *dstattrs, &oldnum, &oldattrs)) != CSA_SUCCESS)
		return (stat);

	if (caltbl == _B_TRUE)
		checknum = CSA_X_DT_CAL_ATTR_DATA_VERSION_I;
	else
		checknum = CSA_X_DT_ENTRY_ATTR_SEQUENCE_END_DATE_I;

	for (i = 0; i < numsrc && stat == CSA_SUCCESS; i++) {
		if (srcattrs[i].name.name == NULL)
			continue;

		if (srcattrs[i].name.num > checknum &&
		     srcattrs[i].name.num <= (*tbl)->size) {

			/* The check is for the case where
			 * the server's hash numbers is different than
			 * ours.  This happens when custom attributes
			 * were introduced before we added to our list
			 * of defined attributes.
			 */ 

			srcattrs[i].name.num = _DtCm_get_index_from_table(*tbl,
						srcattrs[i].name.name);
		}

		if (srcattrs[i].name.num <= 0 ||
		    srcattrs[i].name.num > (*tbl)->size) {

			if ((stat = _DtCmExtendNameTable(srcattrs[i].name.name,
			    srcattrs[i].name.num, 0,
			    (caltbl == _B_TRUE ? _DtCm_cal_name_tbl :
			    _DtCm_entry_name_tbl),
			    (caltbl == _B_TRUE ? _DtCM_DEFINED_CAL_ATTR_SIZE :
			    _DtCM_DEFINED_ENTRY_ATTR_SIZE),
			    (caltbl == _B_TRUE ? _CSA_CALENDAR_ATTRIBUTE_NAMES :
			    _CSA_ENTRY_ATTRIBUTE_NAMES), tbl, NULL))
			    == CSA_SUCCESS) {

				if (srcattrs[i].name.num <= 0)
					srcattrs[i].name.num = (*tbl)->size;

				stat = _DtCmGrowAttrArray(numdst,
					dstattrs, &srcattrs[i]);
			}
		} else {
			if ((*tbl)->names[srcattrs[i].name.num] == NULL) {

				/* fill in the missing hole */
				if ((stat = _DtCm_add_name_to_table(*tbl,
				    srcattrs[i].name.num,
				    srcattrs[i].name.name)) != CSA_SUCCESS)
					break;
			}

			if (srcattrs[i].name.num > *numdst)
				stat = _DtCmGrowAttrArray(numdst, dstattrs,
					&srcattrs[i]);
			else
				stat = _DtCmUpdateAttribute(&srcattrs[i],
					&(*dstattrs)[srcattrs[i].name.num]);
		}
	}

	if (makecopy && oldnum > 0) {
		if (stat != CSA_SUCCESS) {
			_DtCm_free_cms_attributes(*numdst + 1, *dstattrs);
			free(*dstattrs);
			*numdst = oldnum;
			*dstattrs = oldattrs;
		} else {
			_DtCm_free_cms_attributes(oldnum + 1, oldattrs);
			free(oldattrs);
		}
	}

	return (stat);
}

/******************************************************************************
 * static functions used within the file
 ******************************************************************************/

/*
 * The passed in attributes are hashed.
 * For each attribute, check
 * 1. type is valid and supported
 * 2. if it's a date time value type, check validity of date time.
 * 3. if it's a reminder value type, check validity of lead time.
 * 4. if it is a defined attribute, check that the data type is correct.
 * 5. if it is a defined attribute and checkreadonly is set, check
 *	that it's not readonly.
 */
static CSA_return_code
check_predefined_attrs(
	int			fver,
	CSA_uint32		num_attrs,
	cms_attribute		*attrs,
	boolean_t		checkreadonly,
	_DtCmNameTable		*tbl,
	CSA_uint32		num_defined,
	_DtCmAttrInfo		*our_attrs)
{
	int		i, index, cl;
	CSA_reminder	*rptr;
	time_t		tick;

	for (i = 0; i < num_attrs; i++) {

		if (attrs[i].name.name == NULL)
			continue;

		if (tbl == NULL)
			index = i;
		else
			index = _DtCm_get_index_from_table(tbl,
					attrs[i].name.name); 

		if (index > 0 && index <= num_defined) {

			/* check whether the attribute is supported
			 * in this version
			 */
			if (our_attrs[index].fst_vers == 0 ||
			    (fver < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
			    fver < our_attrs[index].fst_vers))
				return (CSA_E_UNSUPPORTED_ATTRIBUTE);
			else if (our_attrs[index].fst_vers == -1)
				return (CSA_E_INVALID_ATTRIBUTE);

			/* check whether the attribute is readonly */
			if (checkreadonly &&
			    ((fver < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
			    our_attrs[index].nex_ro) ||
			    (fver >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION &&
			    our_attrs[index].ex_ro)))
				return (CSA_E_READONLY);

			/* check data type */
			if (attrs[i].value &&
			    attrs[i].value->type != our_attrs[index].type)
				return (CSA_E_INVALID_ATTRIBUTE_VALUE);

			if (index == CSA_ENTRY_ATTR_CLASSIFICATION_I) {
				cl = attrs[i].value->item.uint32_value;
				if (cl < CSA_CLASS_PUBLIC ||
				    cl > CSA_CLASS_CONFIDENTIAL)
					return (CSA_E_INVALID_ATTRIBUTE_VALUE);
			}

			if (index == CSA_ENTRY_ATTR_TYPE_I) {
				cl = attrs[i].value->item.uint32_value;
				if (cl < CSA_TYPE_EVENT ||
				    cl > CSA_X_DT_TYPE_OTHER)
					return (CSA_E_INVALID_ATTRIBUTE_VALUE);
			}

		} else if (fver < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
			return (CSA_E_INVALID_ATTRIBUTE);
		}

		/* check validity of value type */
		if (attrs[i].value) {
			if (attrs[i].value->type < CSA_VALUE_BOOLEAN ||
			    attrs[i].value->type > CSA_VALUE_OPAQUE_DATA)
				return (CSA_E_INVALID_ATTRIBUTE_VALUE);

			/* cast the sint32_value element to the desired
			 * type to be safe since the value part could
			 * actually be a pointer to a CSA_attribute_value
			 */
			switch (attrs[i].value->type) {
			case CSA_VALUE_DATE_TIME:
				if (attrs[i].value->item.sint32_value == 0
				    || _csa_iso8601_to_tick(
				    (char *)attrs[i].value->item.sint32_value,
				    &tick))
					return (CSA_E_INVALID_DATE_TIME);
				break;
			case CSA_VALUE_REMINDER:
				rptr = (CSA_reminder *)
					attrs[i].value->item.sint32_value;
				if (rptr == NULL || rptr->lead_time == NULL ||
				    _csa_iso8601_to_duration(rptr->lead_time,
							     &tick))
					return (CSA_E_INVALID_ATTRIBUTE_VALUE);
				break;
			case CSA_VALUE_ATTENDEE_LIST:
				return (CSA_E_INVALID_ATTRIBUTE_VALUE);
			}
		}
	}

	return (CSA_SUCCESS);
}

static CSA_return_code
convert_cms_user_to_csa_user(char *from, CSA_calendar_user **to)
{
	CSA_calendar_user *newval;

	if ((newval = (CSA_calendar_user *)calloc(1,
	    sizeof(CSA_calendar_user))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (from) {
		if ((newval->user_name = strdup(from)) == NULL) {
			free(newval);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
	}

	*to = newval;
	return (CSA_SUCCESS);
}

static CSA_return_code
hash_entry_attrs(
	CSA_uint32	num_attrs,
	CSA_attribute	*csaattrs,
	cms_attribute	*cmsattrs,
	CSA_uint32	*hnum,
	cms_attribute	**hattrs,
	CSA_uint32	*num)
{
	CSA_return_code		stat;
	int			i, j, index, count = 0;
	cms_attribute		*nattrs;
	char			*name;
	cms_attribute_value	*val;

	if ((nattrs = (cms_attribute *)calloc(1,
	    sizeof(cms_attribute)*(num_attrs+_DtCM_DEFINED_ENTRY_ATTR_SIZE+1)))
	    == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0, j = _DtCM_DEFINED_ENTRY_ATTR_SIZE + 1; i < num_attrs; i++) {
		name = (csaattrs ? csaattrs[i].name : cmsattrs[i].name.name);
		if (name == NULL)
			continue;
		else {
			count++;
			val = (csaattrs ?
				((cms_attribute_value *)csaattrs[i].value) :
				cmsattrs[i].value);
		}

		index = _DtCm_get_index_from_table(_DtCm_entry_name_tbl, name);

		if (cmsattrs) cmsattrs[i].name.num = index;

		if (index > 0) {
			nattrs[index].name.name = name;
			nattrs[index].value = val;
		} else {
			nattrs[j].name.name = name;
			nattrs[j++].value = val;
		}
	}

	if (num) *num = count;
	*hnum = j - 1;
	*hattrs = nattrs;
	return (CSA_SUCCESS);
}

static CSA_return_code
hash_cal_attrs(
	CSA_uint32	num_attrs,
	CSA_attribute	*csaattrs,
	cms_attribute	*cmsattrs,
	CSA_uint32	*hnum,
	cms_attribute	**hattrs,
	CSA_uint32	*num)
{
	CSA_return_code		stat;
	int			i, j, index, count = 0;
	cms_attribute		*nattrs;
	char			*name;
	cms_attribute_value	*val;

	if ((nattrs = (cms_attribute *)calloc(1,
	    sizeof(cms_attribute)*(num_attrs+_DtCM_DEFINED_CAL_ATTR_SIZE+1)))
	    == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	for (i = 0, j = _DtCM_DEFINED_CAL_ATTR_SIZE + 1; i < num_attrs; i++) {
		name = (csaattrs ? csaattrs[i].name : cmsattrs[i].name.name);
		if (name == NULL)
			continue;
		else {
			count++;
			val = (csaattrs ?
				((cms_attribute_value *)csaattrs[i].value) :
				cmsattrs[i].value);
		}

		index = _DtCm_get_index_from_table(_DtCm_cal_name_tbl, name);

		if (cmsattrs) cmsattrs[i].name.num = index;

		if (index > 0) {
			nattrs[index].name.name = name;
			nattrs[index].value = val;
		} else {
			nattrs[j].name.name = name;
			nattrs[j++].value = val;
		}
	}

	if (num) *num = count;
	*hnum = j - 1;
	*hattrs = nattrs;
	return (CSA_SUCCESS);
}

static CSA_return_code
_DtCm_check_hashed_cal_attributes(
	int		fvers,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	char		*owner,
	char		*cname,
	boolean_t	checkreadonly,
	boolean_t	firsttime,
	boolean_t	csatype)
{
	CSA_return_code stat;
	CSA_attribute_value	*csaval;
	cms_attribute_value	*cmsval;
	char			*nattr = NULL; /* calendar name */
	char			*oattr = NULL; /* calendar owner */
	char			*cattr = NULL; /* character set */
	char			*tattr = NULL; /* time zone */

	if (firsttime) {
		if (attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].name.name) {
			if (attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].value == NULL
			    || attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].value->\
			    item.calendar_user_value == NULL)
				return (CSA_E_INVALID_ATTRIBUTE_VALUE);

			if (csatype) {
				csaval = (CSA_attribute_value *)attrs[\
					CSA_CAL_ATTR_CALENDAR_OWNER_I].value;
				stat = _CheckCalendarOwner(owner, cname,
				    csaval->type,
				    (csaval && csaval->item.calendar_user_value?
				    csaval->item.calendar_user_value->user_name:
				    NULL));
			} else {
				cmsval = attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].\
					value;
				stat = _CheckCalendarOwner(owner, cname,
					cmsval->type, cmsval ?
					cmsval->item.calendar_user_value:NULL);
			}

			if (stat != CSA_SUCCESS)
				return (stat);

			oattr = attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].name.name;
			attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].name.name = NULL;
		}

		if (attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].name.name) {
			if (attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].value == NULL ||
			    attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].value->\
			    item.string_value == NULL) {
				if (oattr)
					attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].\
						name.name = oattr;
				return (CSA_E_INVALID_ATTRIBUTE_VALUE);
			}

			if ((stat = _CheckCalendarName(owner, cname,
			    attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].value))
			    != CSA_SUCCESS) {
				if (oattr)
					attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].\
						name.name = oattr;
				return (stat);
			}

			nattr = attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].name.name;
			attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].name.name = NULL;
		}

		if (attrs[CSA_CAL_ATTR_CHARACTER_SET_I].name.name) {
			if (fvers < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION)
				return (CSA_E_INVALID_ATTRIBUTE);

			cattr = attrs[CSA_CAL_ATTR_CHARACTER_SET_I].name.name;
			attrs[CSA_CAL_ATTR_CHARACTER_SET_I].name.name = NULL;
		}

		if (attrs[CSA_CAL_ATTR_TIME_ZONE_I].name.name) {
			if (fvers < _DtCM_FIRST_EXTENSIBLE_DATA_VERSION)
				return (CSA_E_INVALID_ATTRIBUTE);

			tattr = attrs[CSA_CAL_ATTR_TIME_ZONE_I].name.name;
			attrs[CSA_CAL_ATTR_TIME_ZONE_I].name.name = NULL;
		}
	}

	stat = check_predefined_attrs(fvers, num_attrs+1, attrs,
		checkreadonly, NULL, _DtCM_DEFINED_CAL_ATTR_SIZE,
		_CSA_cal_attr_info);

	if (oattr)
		attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].name.name = oattr;

	if (nattr)
		attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].name.name = nattr;

	if (cattr)
		attrs[CSA_CAL_ATTR_CHARACTER_SET_I].name.name = oattr;

	if (tattr)
		attrs[CSA_CAL_ATTR_TIME_ZONE_I].name.name = nattr;

	return (stat);
}

static CSA_return_code
_DtCm_check_hashed_entry_attributes(
	int		fvers,
	CSA_uint32	num_attrs,
	cms_attribute	*attrs,
	CSA_flags	utype)
{
	CSA_return_code stat;

	if ((stat = check_predefined_attrs(fvers, num_attrs+1, attrs,
	    (utype == 0 ? _B_FALSE : _B_TRUE), NULL,
	    _DtCM_DEFINED_ENTRY_ATTR_SIZE, _CSA_entry_attr_info))
	    != CSA_SUCCESS) {

		return (stat);
	}

	if (utype == CSA_CB_ENTRY_ADDED) {

		/* make sure the minimum set of attribute is specified */
		if (attrs[CSA_ENTRY_ATTR_START_DATE_I].value == NULL ||
		    attrs[CSA_ENTRY_ATTR_TYPE_I].value == NULL)
			return (CSA_E_INVALID_PARAMETER);

	} else if (utype == CSA_CB_ENTRY_UPDATED) {

		/* type can only be set at insertion time */
		if (attrs[CSA_ENTRY_ATTR_TYPE_I].name.name)
			return (CSA_E_READONLY);
	}

	return (CSA_SUCCESS);
}

static CSA_return_code
_CheckNameAtHost(char *name, char *value, boolean_t strict)
{
	char		*ptr, *nptr;
	boolean_t	res, isroot;

	/* check name part first */
	if (name == NULL) {
		/* if no name is given, get user name of user running
		 * the application
		 */
		if ((name = _DtCmGetUserName()) == NULL)
			return (CSA_E_FAILURE);
	}

	if (nptr = strchr(name, '@')) *nptr = NULL;
	if (ptr = strchr(value, '@')) *ptr = NULL;
	isroot = !strcmp(name, "root");
	res = isroot || (!strcmp(value, name));
	if (nptr) *nptr = '@';
	if (ptr) *ptr = '@';

	/* name is not root and value does not match name */
	if (res == _B_FALSE)
		return (CSA_E_INVALID_ATTRIBUTE_VALUE);

	if (ptr == NULL)
		return (CSA_SUCCESS);

	if (strict == _B_TRUE && isroot == _B_FALSE) {
		/* check host too if it's specified */
		ptr++;
		if (strcmp(ptr, (nptr ? ++nptr : _DtCmGetLocalHost())) == 0)
			return (CSA_SUCCESS);
		else
			return (CSA_E_INVALID_ATTRIBUTE_VALUE);
	} else
		return (CSA_SUCCESS);
}

/*
 * check the data type and validity of calendar owner attribute
 */
static CSA_return_code
_CheckCalendarOwner(char *user, char *cname, int type, char *value)
{
	CSA_return_code	stat;
	char		*cptr, *vptr;

	if (type != CSA_VALUE_CALENDAR_USER || value == NULL)
		return (CSA_E_INVALID_ATTRIBUTE_VALUE);

	/* make sure value contains the same user name as user
	 * unless user is root
	 */
	if ((stat = _CheckNameAtHost(user, value, _B_FALSE)) != CSA_SUCCESS)
		return (stat);

	/* if calendar name is a user name, make sure the name in
	 * value is the same as the calendar name
	 * i.e., userA cannot own a calendar called "userB" if
	 * userB is a user name.
	 */
	if (cptr = strchr(cname, '@')) *cptr = NULL;
	if (vptr = strchr(value, '@')) *vptr = NULL;
	if (_DtCmIsUserName(cname))
		stat = strcmp(cname, value) ? CSA_E_INVALID_ATTRIBUTE_VALUE :
			CSA_SUCCESS;
	if (cptr) *cptr = '@';
	if (vptr) *vptr = '@';

	return (stat);
}

/*
 * check the data type and validity of calendar name attribute
 */
static CSA_return_code
_CheckCalendarName(char *user, char *cname, cms_attribute_value *val)
{
	CSA_return_code stat;
	char		*ptr;

	if (val->type != CSA_VALUE_STRING)
		return (CSA_E_INVALID_ATTRIBUTE_VALUE);

	/* check that the attribute value is the same as the given
	 * calendar name
	 */
	if ((stat = _CheckNameAtHost(cname, val->item.string_value, _B_TRUE))
	    != CSA_SUCCESS)
		return (stat);

	/* now make sure if cal name is a user name, it's
	 * the same as that of the calling user unelss root is doing
	 * the operation
	 */
	if (ptr = strchr(cname, '@')) *ptr = NULL;
	if (_DtCmIsUserName(cname))
		stat = _CheckNameAtHost(user, cname, _B_FALSE);
	else
		stat = CSA_SUCCESS;
	if (ptr) *ptr = '@';

	return (stat);
}

