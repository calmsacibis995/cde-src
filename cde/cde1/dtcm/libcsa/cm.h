/* $XConsortium: cm.h /main/cde1_maint/2 1995/10/10 13:28:05 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CM_H
#define _CM_H

#pragma ident "@(#)cm.h	1.12 97/01/31 Sun Microsystems, Inc."

#ifndef SunOS
#include <rpc/types.h>
#endif /* HPUX */

#include <rpc/rpc.h>

#ifdef __cplusplus
extern "C" {
#endif


#include "csa.h" 
#include "connection.h"


/*
 * cms_name - a user name or calendar name which is
 *		variable length character string
 */

typedef char *cms_name;

/*
 * buffer - a variable length character string
 */

typedef char *buffer;

/*
 * cms_access_entry - data structure for the access list attribute
 */

struct cms_access_entry {
	cms_name user;
	CSA_flags rights;
	struct cms_access_entry *next;
};
typedef struct cms_access_entry cms_access_entry;

/*
 * cms_attribute_value - data structure for attribute value
 */
struct cms_attribute_value {
	CSA_enum	type;
	union {
		CSA_boolean	boolean_value;
		CSA_enum	enumerated_value;
		CSA_flags	flags_value;
		CSA_sint32	sint32_value;
		CSA_uint32	uint32_value;
		char		*string_value;
		char		*calendar_user_value;
		char		*date_time_value;
		char		*date_time_range_value;
		char		*time_duration_value;
		cms_access_entry *access_list_value;
		CSA_date_time_entry *date_time_list_value;
		CSA_reminder	*reminder_value;
		CSA_opaque_data *opaque_data_value;
	} item;
};
typedef struct cms_attribute_value cms_attribute_value;


/*
 * cms_key consists of the time and the unique
 * identifier of the entry.
 */

struct cms_key {
	time_t time;
	long id;
};
typedef struct cms_key cms_key;

/*
 * num contains the hashed number for the associated name assigned
 * by the server.  Client should not change its value.
 * If it's zero, name is not hashed yet.
 */

struct cms_attr_name {
	short num;
	cms_name name;
};
typedef struct cms_attr_name cms_attr_name;

/*
 *
 */

struct cms_attribute {
	cms_attr_name name;
	cms_attribute_value *value;
};
typedef struct cms_attribute cms_attribute;


/*
 * An entry is identified by an identifier unique within the calendar
 * and has a variable number of attributes associated with it.
 */

struct cms_entry {
	cms_key key;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
	struct cms_entry *next;
};
typedef struct cms_entry cms_entry;


/*
 * cms_enumerate_calendar_attr_res
 *
 * If stat is CSA_SUCCESS, names contains an array of names.
 */

struct cms_enumerate_calendar_attr_res {
	CSA_return_code stat;
	CSA_uint32 num_names;
	cms_attr_name *names;
};
typedef struct cms_enumerate_calendar_attr_res cms_enumerate_calendar_attr_res;


/*
 * cms_list_calendars_res
 *
 * If stat is CSA_SUCCESS, names contains an array of names.
 */

struct cms_list_calendars_res {
	CSA_return_code stat;
	CSA_uint32 num_names;
	cms_name *names;
};
typedef struct cms_list_calendars_res cms_list_calendars_res;


/*
 * cms_open_args - used by CMS_OPEN_CALENDAR
 */

struct cms_open_args {
	cms_name cal;
	pid_t pid;
};
typedef struct cms_open_args cms_open_args;


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

struct cms_open_res {
	CSA_return_code stat;
	int svr_vers;
	int file_vers;
	CSA_flags user_access;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
};
typedef struct cms_open_res cms_open_res;


/*
 * cms_create_args - used by CMS_CREATE_CALENDAR
 */

struct cms_create_args {
	cms_name cal;
	buffer char_set;
	pid_t pid;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
};
typedef struct cms_create_args cms_create_args;


/*
 * cms_remove_args - used by CMS_REMOVE_CALENDAR
 */

struct cms_remove_args {
	cms_name cal;
	pid_t pid;
};
typedef struct cms_remove_args cms_remove_args;


/*
 * cms_register_args - used by CMS_REGISTER and CMS_UNREGISTER
 */

struct cms_register_args {
	cms_name cal;
	CSA_flags update_type;
	u_long prognum;
	u_long versnum;
	u_long procnum;
	pid_t pid;
};
typedef struct cms_register_args cms_register_args;


/*
 * cms_get_cal_attr_res - used by CMS_GET_CALENDAR_ATTR
 *
 * If stat is CSA_SUCCESS, attrs contains an array of attributes.
 */

struct cms_get_cal_attr_res {
	CSA_return_code stat;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
};
typedef struct cms_get_cal_attr_res cms_get_cal_attr_res;


/*
 * cms_get_cal_attr_args - used by CMS_GET_CALENDAR_ATTR
 */

struct cms_get_cal_attr_args {
	cms_name cal;
	CSA_uint32 num_names;
	cms_attr_name *names;
};
typedef struct cms_get_cal_attr_args cms_get_cal_attr_args;


/*
 * cms_set_cal_attr_args - used by CMS_SET_CALENDAR_ATTR
 */

struct cms_set_cal_attr_args {
	cms_name cal;
	pid_t pid;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
};
typedef struct cms_set_cal_attr_args cms_set_cal_attr_args;


/*
 * cms_archive_res - used by CMS_ARCHIVE
 *
 * If stat is CSA_SUCCESS, data contains the data to be archived
 */

struct cms_archive_res {
	CSA_return_code stat;
	buffer data;
};
typedef struct cms_archive_res cms_archive_res;


/*
 * cms_archive_args - used by CMS_ARCHIVE
 */

struct cms_archive_args {
	cms_name cal;
	bool_t delete;
	buffer char_set;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
	CSA_enum *ops;
};
typedef struct cms_archive_args cms_archive_args;


/*
 * cms_restore_args - used by CMS_RESTORE
 */

struct cms_restore_args {
	cms_name cal;
	buffer data;
	buffer char_set;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
	CSA_enum *ops;
};
typedef struct cms_restore_args cms_restore_args;


/*
 * cms_reminder
 */

struct cms_reminder_ref {
	buffer reminder_name;
	buffer entryid;
	cms_key	key;
	time_t	runtime;
	struct cms_reminder_ref *next;
};
typedef struct cms_reminder_ref cms_reminder_ref;


/*
 * cms_reminder_res - used by CMS_LOOKUP_REMINDER
 *
 * If stat is CSA_SUCCESS, rems contains an array of reminders.
 */

struct cms_reminder_res {
	CSA_return_code stat;
	cms_reminder_ref *rems;
};
typedef struct cms_reminder_res cms_reminder_res;


/*
 * cms_reminder_args - used by CMS_LOOKUP_REMINDER
 */

struct cms_reminder_args {
	cms_name cal;
	time_t tick;
	CSA_uint32 num_names;
	cms_attr_name *names;
};
typedef struct cms_reminder_args cms_reminder_args;


/*
 * cms_entries_res
 *
 * If stat is CSA_SUCCESS, entries contains an array of entries.
 */

struct cms_entries_res {
	CSA_return_code stat;
	cms_entry *entries;
};
typedef struct cms_entries_res cms_entries_res;


/*
 * cms_lookup_entries_args - used by CMS_LOOKUP_ENTRIES
 */

struct cms_lookup_entries_args {
	cms_name cal;
	buffer char_set;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
	CSA_enum *ops;
};
typedef struct cms_lookup_entries_args cms_lookup_entries_args;


/*
 * cms_enumerate_args - used by CMS_ENUERATE_SEQUENCE
 */

struct cms_enumerate_args {
	cms_name cal;
	long id;
	time_t start;
	time_t end;
};
typedef struct cms_enumerate_args cms_enumerate_args;

/*
 * If stat is CSA_SUCCESS, attrs contains an array of
 * attributes.
 */

struct cms_get_entry_attr_res_item {
	CSA_return_code stat;
	cms_key key;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
	struct cms_get_entry_attr_res_item *next;
};
typedef struct cms_get_entry_attr_res_item cms_get_entry_attr_res_item;


/*
 * cms_get_entry_attr_res - used by CMS_GET_ENTRY_ATTR
 *
 * If stat is CSA_SUCCESS, entries contains an array of
 * cms_get_entry_attr_res_item structures.
 */

struct cms_get_entry_attr_res {
	CSA_return_code stat;
	cms_get_entry_attr_res_item *entries;
};
typedef struct cms_get_entry_attr_res cms_get_entry_attr_res;


/*
 * cms_get_entry_attr_args - used by CMS_GET_ENTRY_ATTR
 */

struct cms_get_entry_attr_args {
	cms_name cal;
	CSA_uint32 num_keys;
	cms_key *keys;
	CSA_uint32 num_names;
	cms_attr_name *names;
};
typedef struct cms_get_entry_attr_args cms_get_entry_attr_args;


/*
 * cms_entry_res
 *
 * If stat is CSA_SUCCESS, entry points to an entry.
 */

struct cms_entry_res {
	CSA_return_code stat;
	cms_entry *entry;
};
typedef struct cms_entry_res cms_entry_res;


/*
 * cms_insert_args - used by CMS_INSERT_ENTRY
 */

struct cms_insert_args {
	cms_name cal;
	pid_t pid;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
};
typedef struct cms_insert_args cms_insert_args;


/*
 * cms_update_args - used by CMS_UPDATE_ENTRY
 */

struct cms_update_args {
	cms_name cal;
	pid_t pid;
	cms_key entry;
	CSA_enum scope;
	CSA_uint32 num_attrs;
	cms_attribute *attrs;
};
typedef struct cms_update_args cms_update_args;


/*
 * cms_delete_args - used by CMS_DELETE_ENTRY
 */

struct cms_delete_args {
	cms_name cal;
	pid_t pid;
	cms_key entry;
	CSA_enum scope;
};
typedef struct cms_delete_args cms_delete_args;

#define	TABLEPROG ((unsigned long)(100068))
#define	TABLEVERS ((unsigned long)(5))

#define	CMS_PING ((unsigned long)(0))
extern  enum clnt_stat cms_ping_5(void *, void *, _DtCm_Connection *);
extern  bool_t cms_ping_5_svc(void *, void *, struct svc_req *);
#define	CMS_LIST_CALENDARS ((unsigned long)(1))
extern  enum clnt_stat cms_list_calendars_5(void *, cms_list_calendars_res *,_DtCm_Connection *);
extern  bool_t cms_list_calendars_5_svc(void *, cms_list_calendars_res *, struct svc_req *);
#define	CMS_OPEN_CALENDAR ((unsigned long)(2))
extern  enum clnt_stat cms_open_calendar_5(cms_open_args *, cms_open_res *, _DtCm_Connection *);
extern  bool_t cms_open_calendar_5_svc(cms_open_args *, cms_open_res *, struct svc_req *);
#define	CMS_CREATE_CALENDAR ((unsigned long)(3))
extern  enum clnt_stat cms_create_calendar_5(cms_create_args *, CSA_return_code *, _DtCm_Connection *);
extern  bool_t cms_create_calendar_5_svc(cms_create_args *, CSA_return_code *, struct svc_req *);
#define	CMS_REMOVE_CALENDAR ((unsigned long)(4))
extern  enum clnt_stat cms_remove_calendar_5(cms_remove_args *, CSA_return_code *, _DtCm_Connection *);
extern  bool_t cms_remove_calendar_5_svc(cms_remove_args *, CSA_return_code *, struct svc_req *);
#define	CMS_REGISTER ((unsigned long)(5))
extern  enum clnt_stat cms_register_5(cms_register_args *, CSA_return_code *, _DtCm_Connection *);
extern  bool_t cms_register_5_svc(cms_register_args *, CSA_return_code *, struct svc_req *);
#define	CMS_UNREGISTER ((unsigned long)(6))
extern  enum clnt_stat cms_unregister_5(cms_register_args *, CSA_return_code *, _DtCm_Connection *);
extern  bool_t cms_unregister_5_svc(cms_register_args *, CSA_return_code *, struct svc_req *);
#define	CMS_ENUMERATE_CALENDAR_ATTR ((unsigned long)(7))
extern  enum clnt_stat cms_enumerate_calendar_attr_5(cms_name *, cms_enumerate_calendar_attr_res *, _DtCm_Connection *);
extern  bool_t cms_enumerate_calendar_attr_5_svc(cms_name *, cms_enumerate_calendar_attr_res *, struct svc_req *);
#define	CMS_GET_CALENDAR_ATTR ((unsigned long)(8))
extern  enum clnt_stat cms_get_calendar_attr_5(cms_get_cal_attr_args *, cms_get_cal_attr_res *, _DtCm_Connection *);
extern  bool_t cms_get_calendar_attr_5_svc(cms_get_cal_attr_args *, cms_get_cal_attr_res *, struct svc_req *);
#define	CMS_SET_CALENDAR_ATTR ((unsigned long)(9))
extern  enum clnt_stat cms_set_calendar_attr_5(cms_set_cal_attr_args *, CSA_return_code *, _DtCm_Connection *);
extern  bool_t cms_set_calendar_attr_5_svc(cms_set_cal_attr_args *, CSA_return_code *, struct svc_req *);
#define	CMS_ARCHIVE ((unsigned long)(10))
extern  enum clnt_stat cms_archive_5(cms_archive_args *, cms_archive_res *, _DtCm_Connection *);
extern  bool_t cms_archive_5_svc(cms_archive_args *, cms_archive_res *, struct svc_req *);
#define	CMS_RESTORE ((unsigned long)(11))
extern  enum clnt_stat cms_restore_5(cms_restore_args *, CSA_return_code *, _DtCm_Connection *);
extern  bool_t cms_restore_5_svc(cms_restore_args *, CSA_return_code *, struct svc_req *);
#define	CMS_LOOKUP_REMINDER ((unsigned long)(12))
extern  enum clnt_stat cms_lookup_reminder_5(cms_reminder_args *, cms_reminder_res *, _DtCm_Connection *);
extern  bool_t cms_lookup_reminder_5_svc(cms_reminder_args *, cms_reminder_res *, struct svc_req *);
#define	CMS_LOOKUP_ENTRIES ((unsigned long)(13))
extern  enum clnt_stat cms_lookup_entries_5(cms_lookup_entries_args *, cms_entries_res *, _DtCm_Connection *);
extern  bool_t cms_lookup_entries_5_svc(cms_lookup_entries_args *, cms_entries_res *, struct svc_req *);
#define	CMS_ENUMERATE_SEQUENCE ((unsigned long)(14))
extern  enum clnt_stat cms_enumerate_sequence_5(cms_enumerate_args *, cms_entries_res *, _DtCm_Connection *);
extern  bool_t cms_enumerate_sequence_5_svc(cms_enumerate_args *, cms_entries_res *, struct svc_req *);
#define	CMS_GET_ENTRY_ATTR ((unsigned long)(15))
extern  enum clnt_stat cms_get_entry_attr_5(cms_get_entry_attr_args *, cms_get_entry_attr_res *, _DtCm_Connection *);
extern  bool_t cms_get_entry_attr_5_svc(cms_get_entry_attr_args *, cms_get_entry_attr_res *, struct svc_req *);
#define	CMS_INSERT_ENTRY ((unsigned long)(16))
extern  enum clnt_stat cms_insert_entry_5(cms_insert_args *, cms_entry_res *, _DtCm_Connection *);
extern  bool_t cms_insert_entry_5_svc(cms_insert_args *, cms_entry_res *, struct svc_req *);
#define	CMS_UPDATE_ENTRY ((unsigned long)(17))
extern  enum clnt_stat cms_update_entry_5(cms_update_args *, cms_entry_res *, _DtCm_Connection *);
extern  bool_t cms_update_entry_5_svc(cms_update_args *, cms_entry_res *, struct svc_req *);
#define	CMS_DELETE_ENTRY ((unsigned long)(18))
extern  enum clnt_stat cms_delete_entry_5(cms_delete_args *, CSA_return_code *, _DtCm_Connection *);
extern  bool_t cms_delete_entry_5_svc(cms_delete_args *, CSA_return_code *, struct svc_req *);
extern int tableprog_5_freeresult(SVCXPRT *, xdrproc_t, caddr_t);

/* the xdr functions */

extern  bool_t xdr_cms_name(XDR *, cms_name*);
extern  bool_t xdr_buffer(XDR *, buffer*);
extern  bool_t xdr_cms_access_entry(XDR *, cms_access_entry*);
extern  bool_t xdr_cms_key(XDR *, cms_key*);
extern  bool_t xdr_cms_attr_name(XDR *, cms_attr_name*);
extern  bool_t xdr_cms_attribute(XDR *, cms_attribute*);
extern  bool_t xdr_cms_entry(XDR *, cms_entry*);
extern  bool_t xdr_cms_enumerate_calendar_attr_res(XDR *, cms_enumerate_calendar_attr_res*);
extern  bool_t xdr_cms_list_calendars_res(XDR *, cms_list_calendars_res*);
extern  bool_t xdr_cms_open_args(XDR *, cms_open_args*);
extern  bool_t xdr_cms_open_res(XDR *, cms_open_res*);
extern  bool_t xdr_cms_create_args(XDR *, cms_create_args*);
extern  bool_t xdr_cms_remove_args(XDR *, cms_remove_args*);
extern  bool_t xdr_cms_register_args(XDR *, cms_register_args*);
extern  bool_t xdr_cms_get_cal_attr_res(XDR *, cms_get_cal_attr_res*);
extern  bool_t xdr_cms_get_cal_attr_args(XDR *, cms_get_cal_attr_args*);
extern  bool_t xdr_cms_set_cal_attr_args(XDR *, cms_set_cal_attr_args*);
extern  bool_t xdr_cms_archive_res(XDR *, cms_archive_res*);
extern  bool_t xdr_cms_archive_args(XDR *, cms_archive_args*);
extern  bool_t xdr_cms_restore_args(XDR *, cms_restore_args*);
extern  bool_t xdr_cms_reminder_ref(XDR *, cms_reminder_ref*);
extern  bool_t xdr_cms_reminder_res(XDR *, cms_reminder_res*);
extern  bool_t xdr_cms_reminder_args(XDR *, cms_reminder_args*);
extern  bool_t xdr_cms_entries_res(XDR *, cms_entries_res*);
extern  bool_t xdr_cms_lookup_entries_args(XDR *, cms_lookup_entries_args*);
extern  bool_t xdr_cms_enumerate_args(XDR *, cms_enumerate_args*);
extern  bool_t xdr_cms_get_entry_attr_res_item(XDR *, cms_get_entry_attr_res_item*);
extern  bool_t xdr_cms_get_entry_attr_res(XDR *, cms_get_entry_attr_res*);
extern  bool_t xdr_cms_get_entry_attr_args(XDR *, cms_get_entry_attr_args*);
extern  bool_t xdr_cms_entry_res(XDR *, cms_entry_res*);
extern  bool_t xdr_cms_insert_args(XDR *, cms_insert_args*);
extern  bool_t xdr_cms_update_args(XDR *, cms_update_args*);
extern  bool_t xdr_cms_delete_args(XDR *, cms_delete_args*);

#ifdef __cplusplus
}
#endif

#ifdef HPUX
#undef hpux
#define hpux
#endif
#endif

