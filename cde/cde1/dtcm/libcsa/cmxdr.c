/* $XConsortium: cmxdr.c /main/cde1_maint/2 1995/10/10 13:28:43 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)cmxdr.c	1.12 96/11/12 Sun Microsystems, Inc."

/*
 * xdr routines for xapia csa data structures
 */

#include "cm.h"
#include "csa.h" 
#include "cmxdr.h"

bool_t
xdr_CSA_uint32(register XDR *xdrs, CSA_uint32 *objp)
{
	if (!xdr_u_long(xdrs, objp))
		return (FALSE);

	return (TRUE);
}

bool_t
xdr_CSA_sint32(register XDR *xdrs, CSA_sint32 *objp)
{
	if (!xdr_long(xdrs, objp))
		return (FALSE);

	return (TRUE);
}

bool_t
xdr_pid_t(register XDR *xdrs, pid_t *objp)
{
	if (!xdr_long(xdrs, objp))
		return (FALSE);

	return (TRUE);
}

/*
 * time_t is a typedef which is system dependent
 */
bool_t
xdr_time_t(register XDR *xdrs, time_t *objp)
{
#ifdef __osf__
	if (!xdr_int(xdrs, objp))
		return (FALSE);
#else
	if (!xdr_long(xdrs, objp))
		return (FALSE);
#endif
	return (TRUE);
}
 

/*
 * cms_name - a user name or calendar name which is
 *		variable length character string
 */

bool_t
xdr_cms_name(register XDR *xdrs, cms_name *objp)
{

	if (!xdr_string(xdrs, objp, ~0))
		return (FALSE);
	return (TRUE);
}

/*
 * cms_access_entry - data structure for the access list attribute
 */

bool_t
xdr_cms_access_entry(register XDR *xdrs, cms_access_entry *objp)
{

	if (!xdr_string(xdrs, &objp->user, ~0))
		return (FALSE);
	if (!xdr_CSA_uint32(xdrs, &objp->rights))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->next, sizeof (cms_access_entry),
	    (xdrproc_t) xdr_cms_access_entry))
		return (FALSE);
	return (TRUE);
}

/*
 * cms_key consists of the time and the unique
 * identifier of the entry.
 */

bool_t
xdr_cms_key(register XDR *xdrs, cms_key *objp)
{

	if (!xdr_time_t(xdrs, &objp->time))
		return (FALSE);
	if (!xdr_long(xdrs, &objp->id))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_CSA_opaque_data(register XDR *xdrs, CSA_opaque_data *objp)
{

	if (!xdr_bytes(xdrs, (char **)&objp->data, (u_int *) &objp->size, ~0))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_CSA_reminder(register XDR *xdrs, CSA_reminder *objp)
{

	if (!xdr_string(xdrs, &objp->lead_time, ~0))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->snooze_time, ~0))
		return (FALSE);
	if (!xdr_CSA_uint32(xdrs, &objp->repeat_count))
		return (FALSE);
	if (!xdr_CSA_opaque_data(xdrs, &objp->reminder_data))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_CSA_date_time_entry(register XDR *xdrs, CSA_date_time_entry *objp)
{

	if (!xdr_string(xdrs, &objp->date_time, ~0))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->next,
	    sizeof (CSA_date_time_entry), (xdrproc_t) xdr_CSA_date_time_entry))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_cms_attribute_value(register XDR *xdrs, cms_attribute_value *objp)
{

	if (!xdr_CSA_sint32(xdrs, &objp->type))
		return (FALSE);

	switch (objp->type) {
	case CSA_VALUE_SINT32:
	case CSA_VALUE_ENUMERATED:
		if (!xdr_CSA_sint32(xdrs, &objp->item.sint32_value))
			return (FALSE);
		break;
	case CSA_VALUE_BOOLEAN:
	case CSA_VALUE_FLAGS:
	case CSA_VALUE_UINT32:
		if (!xdr_CSA_uint32(xdrs, &objp->item.uint32_value))
			return (FALSE);
		break;
	case CSA_VALUE_STRING:
	case CSA_VALUE_CALENDAR_USER:
	case CSA_VALUE_DATE_TIME:
	case CSA_VALUE_DATE_TIME_RANGE:
	case CSA_VALUE_TIME_DURATION:
		if (!xdr_string(xdrs, &objp->item.string_value, ~0))
			return (FALSE);
		break;
	case CSA_VALUE_ACCESS_LIST:
		if (!xdr_pointer(xdrs, (char **)&objp->item.access_list_value,
		    sizeof (cms_access_entry),
		    (xdrproc_t) xdr_cms_access_entry))
			return (FALSE);
		break;
	case CSA_VALUE_DATE_TIME_LIST:
		if (!xdr_pointer(xdrs,
		    (char **)&objp->item.date_time_list_value,
		    sizeof (CSA_date_time_entry),
		    (xdrproc_t) xdr_CSA_date_time_entry))
			return (FALSE);
		break;
	case CSA_VALUE_REMINDER:
		if (!xdr_pointer(xdrs,
		    (char **)&objp->item.reminder_value,
		    sizeof (CSA_reminder), (xdrproc_t) xdr_CSA_reminder))
			return (FALSE);
		break;
	case CSA_VALUE_OPAQUE_DATA:
		if (!xdr_pointer(xdrs, (char **)&objp->item.opaque_data_value,
		    sizeof (CSA_opaque_data), (xdrproc_t) xdr_CSA_opaque_data))
			return (FALSE);
		break;
	}
	return (TRUE);
}

/*
 * num contains the hashed number for the associated name assigned
 * by the server.  Client should not change its value.
 * If it's zero, name is not hashed yet.
 */

bool_t
xdr_cms_attr_name(register XDR *xdrs, cms_attr_name *objp)
{

	if (!xdr_short(xdrs, &objp->num))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->name, ~0))
		return (FALSE);
	return (TRUE);
}

/*
 *
 */

bool_t
xdr_cms_attribute(register XDR *xdrs, cms_attribute *objp)
{

	if (!xdr_cms_attr_name(xdrs, &objp->name))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->value,
	    sizeof (CSA_attribute_value), (xdrproc_t) xdr_cms_attribute_value))
		return (FALSE);
	return (TRUE);
}


/*
 * An entry is identified by an identifier unique within the calendar
 * and has a variable number of attributes associated with it.
 */

bool_t
xdr_cms_entry(register XDR *xdrs, cms_entry *objp)
{

	if (!xdr_cms_key(xdrs, &objp->key))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->next, sizeof (cms_entry),
	    (xdrproc_t) xdr_cms_entry))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_enumerate_calendar_attr_res
 *
 * If stat is CSA_SUCCESS, names contains an array of names.
 */

bool_t
xdr_cms_enumerate_calendar_attr_res(register XDR *xdrs, cms_enumerate_calendar_attr_res *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->names, (u_int *) &objp->num_names,
	    ~0, sizeof (cms_attr_name), (xdrproc_t) xdr_cms_attr_name))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_list_calendars_res
 *
 * If stat is CSA_SUCCESS, names contains an array of names.
 */

bool_t
xdr_cms_list_calendars_res(register XDR *xdrs, cms_list_calendars_res *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->names, (u_int *) &objp->num_names,
	    ~0, sizeof (cms_name), (xdrproc_t) xdr_cms_name))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_open_args - used by CMS_OPEN_CALENDAR
 */

bool_t
xdr_cms_open_args(register XDR *xdrs, cms_open_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_pid_t(xdrs, &objp->pid))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_open_res - returned by CMS_OPEN_CALENDAR
 *
 * If stat is DtCm_OK,
 *	svr_vers contains the version number of the server,
 *	file_vers contains the version number of the callog file,
 *	user_access is the calling user's access right to the calendar
 *	num_attrs indicates the size of the array pointed to by attrs,
 *	and attrs contains an array of calendar attributes.
 */

bool_t
xdr_cms_open_res(register XDR *xdrs, cms_open_res *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->svr_vers))
		return (FALSE);
	if (!xdr_int(xdrs, &objp->file_vers))
		return (FALSE);
	if (!xdr_CSA_uint32(xdrs, &objp->user_access))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_create_args - used by CMS_CREATE_CALENDAR
 */

bool_t
xdr_cms_create_args(register XDR *xdrs, cms_create_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->char_set, ~0))
		return (FALSE);
	if (!xdr_pid_t(xdrs, &objp->pid))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_remove_args - used by CMS_REMOVE_CALENDAR
 */

bool_t
xdr_cms_remove_args(register XDR *xdrs, cms_remove_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_pid_t(xdrs, &objp->pid))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_register_args - used by CMS_REGISTER and CMS_UNREGISTER
 */

bool_t
xdr_cms_register_args(register XDR *xdrs, cms_register_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_CSA_uint32(xdrs, &objp->update_type))
		return (FALSE);
	if (!xdr_u_long(xdrs, &objp->prognum))
		return (FALSE);
	if (!xdr_u_long(xdrs, &objp->versnum))
		return (FALSE);
	if (!xdr_u_long(xdrs, &objp->procnum))
		return (FALSE);
	if (!xdr_pid_t(xdrs, &objp->pid))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_get_cal_attr_res - used by CMS_GET_CALENDAR_ATTR
 *
 * If stat is CSA_SUCCESS, attrs contains an array of attributes.
 */

bool_t
xdr_cms_get_cal_attr_res(register XDR *xdrs, cms_get_cal_attr_res *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_get_cal_attr_args - used by CMS_GET_CALENDAR_ATTR
 */

bool_t
xdr_cms_get_cal_attr_args(register XDR *xdrs, cms_get_cal_attr_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->names, (u_int *) &objp->num_names,
	    ~0, sizeof (cms_attr_name), (xdrproc_t) xdr_cms_attr_name))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_set_cal_attr_args - used by CMS_SET_CALENDAR_ATTR
 */

bool_t
xdr_cms_set_cal_attr_args(register XDR *xdrs, cms_set_cal_attr_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_pid_t(xdrs, &objp->pid))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_archive_res - used by CMS_ARCHIVE
 *
 * If stat is CSA_SUCCESS, data contains the data to be archived
 */

bool_t
xdr_cms_archive_res(register XDR *xdrs, cms_archive_res *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->data, ~0))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_archive_args - used by CMS_ARCHIVE
 */

bool_t
xdr_cms_archive_args(register XDR *xdrs, cms_archive_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_bool(xdrs, &objp->delete))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->char_set, ~0))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->ops, (u_int *) &objp->num_attrs,
	    ~0, sizeof (CSA_enum), (xdrproc_t) xdr_CSA_sint32))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_restore_args - used by CMS_RESTORE
 */

bool_t
xdr_cms_restore_args(register XDR *xdrs, cms_restore_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->data, ~0))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->char_set, ~0))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->ops, (u_int *) &objp->num_attrs,
	    ~0, sizeof (CSA_enum), (xdrproc_t) xdr_CSA_sint32))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_reminder
 */

bool_t
xdr_cms_reminder_ref(register XDR *xdrs, cms_reminder_ref *objp)
{

	if (!xdr_string(xdrs, &objp->reminder_name, ~0))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->entryid, ~0))
		return (FALSE);
	if (!xdr_cms_key(xdrs, &objp->key))
		return (FALSE);
	if (!xdr_time_t(xdrs, &objp->runtime))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->next, sizeof (cms_reminder_ref),
	    (xdrproc_t) xdr_cms_reminder_ref))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_reminder_res - used by CMS_LOOKUP_REMINDER
 *
 * If stat is CSA_SUCCESS, rems contains an array of reminders.
 */

bool_t
xdr_cms_reminder_res(register XDR *xdrs, cms_reminder_res *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->rems, sizeof (cms_reminder_ref),
	    (xdrproc_t) xdr_cms_reminder_ref))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_reminder_args - used by CMS_LOOKUP_REMINDER
 */

bool_t
xdr_cms_reminder_args(register XDR *xdrs, cms_reminder_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_time_t(xdrs, &objp->tick))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->names, (u_int *) &objp->num_names,
	    ~0, sizeof (cms_attr_name), (xdrproc_t) xdr_cms_attr_name))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_entries_res
 *
 * If stat is CSA_SUCCESS, entries contains an array of entries.
 */

bool_t
xdr_cms_entries_res(register XDR *xdrs, cms_entries_res *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->entries, sizeof (cms_entry),
	    (xdrproc_t) xdr_cms_entry))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_lookup_entries_args - used by CMS_LOOKUP_ENTRIES
 */

bool_t
xdr_cms_lookup_entries_args(register XDR *xdrs, cms_lookup_entries_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->char_set, ~0))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->ops, (u_int *) &objp->num_attrs,
	    ~0, sizeof (CSA_enum), (xdrproc_t) xdr_CSA_sint32))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_enumerate_args - used by CMS_ENUERATE_SEQUENCE
 */

bool_t
xdr_cms_enumerate_args(register XDR *xdrs, cms_enumerate_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_long(xdrs, &objp->id))
		return (FALSE);
	if (!xdr_time_t(xdrs, &objp->start))
		return (FALSE);
	if (!xdr_time_t(xdrs, &objp->end))
		return (FALSE);
	return (TRUE);
}

/*
 * If stat is CSA_SUCCESS, attrs contains an array of
 * attributes.
 */

bool_t
xdr_cms_get_entry_attr_res_item(
	register XDR			*xdrs,
	cms_get_entry_attr_res_item	*objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_cms_key(xdrs, &objp->key))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->next,
	    sizeof (cms_get_entry_attr_res_item),
	    (xdrproc_t) xdr_cms_get_entry_attr_res_item))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_get_entry_attr_res - used by CMS_GET_ENTRY_ATTR
 *
 * If stat is CSA_SUCCESS, entries contains an array of
 * cms_get_entry_attr_res_item structures.
 */

bool_t
xdr_cms_get_entry_attr_res(register XDR *xdrs, cms_get_entry_attr_res *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->entries,
	    sizeof (cms_get_entry_attr_res_item),
	    (xdrproc_t) xdr_cms_get_entry_attr_res_item))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_get_entry_attr_args - used by CMS_GET_ENTRY_ATTR
 */

bool_t
xdr_cms_get_entry_attr_args(register XDR *xdrs, cms_get_entry_attr_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->keys, (u_int *) &objp->num_keys,
	    ~0, sizeof (cms_key), (xdrproc_t) xdr_cms_key))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->names, (u_int *) &objp->num_names,
	    ~0, sizeof (cms_attr_name), (xdrproc_t) xdr_cms_attr_name))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_entry_res
 *
 * If stat is CSA_SUCCESS, entry points to an entry.
 */

bool_t
xdr_cms_entry_res(register XDR *xdrs, cms_entry_res *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->stat))
		return (FALSE);
	if (!xdr_pointer(xdrs, (char **)&objp->entry, sizeof (cms_entry),
	    (xdrproc_t) xdr_cms_entry))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_insert_args - used by CMS_INSERT_ENTRY
 */

bool_t
xdr_cms_insert_args(register XDR *xdrs, cms_insert_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_pid_t(xdrs, &objp->pid))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_update_args - used by CMS_UPDATE_ENTRY
 */

bool_t
xdr_cms_update_args(register XDR *xdrs, cms_update_args *objp)
{

	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_pid_t(xdrs, &objp->pid))
		return (FALSE);
	if (!xdr_cms_key(xdrs, &objp->entry))
		return (FALSE);
	if (!xdr_CSA_sint32(xdrs, &objp->scope))
		return (FALSE);
	if (!xdr_array(xdrs, (char **)&objp->attrs, (u_int *) &objp->num_attrs,
	    ~0, sizeof (cms_attribute), (xdrproc_t) xdr_cms_attribute))
		return (FALSE);
	return (TRUE);
}


/*
 * cms_delete_args - used by CMS_DELETE_ENTRY
 */

bool_t
xdr_cms_delete_args(register XDR *xdrs, cms_delete_args *objp)
{
	if (!xdr_string(xdrs, &objp->cal, ~0))
		return (FALSE);
	if (!xdr_pid_t(xdrs, &objp->pid))
		return (FALSE);
	if (!xdr_cms_key(xdrs, &objp->entry))
		return (FALSE);
	if (!xdr_CSA_sint32(xdrs, &objp->scope))
		return (FALSE);
	return (TRUE);
}
