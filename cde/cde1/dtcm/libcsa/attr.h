/* $XConsortium: attr.h /main/cde1_maint/1 1995/07/17 19:48:19 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)attr.h	1.24 96/11/12 Sun Microsystems, Inc."

#ifndef _ATTR_H
#define _ATTR_H

#include "ansi_c.h"
#include "csa.h"
#include "cm.h"
#include "nametbl.h"

/*
 * types of reminder used by old versions
 */

#define _DtCM_OLD_ATTR_BEEP_REMINDER		"bp"
#define _DtCM_OLD_ATTR_FLASH_REMINDER		"fl"
#define _DtCM_OLD_ATTR_MAIL_REMINDER		"ml"
#define _DtCM_OLD_ATTR_POPUP_REMINDER		"op"

/*
 * number of attributes for old and new format calendars
 */
#define _DtCM_OLD_CAL_ATTR_SIZE			9
#define _DtCM_OLD_ENTRY_ATTR_SIZE		19
#define _DtCM_DEFINED_CAL_ATTR_SIZE		16
#define _DtCM_DEFINED_ENTRY_ATTR_SIZE		36

/*
 * "repeat forever" value used in old format calendars
 */
#define _DtCM_OLD_REPEAT_FOREVER		999999999

/*
 * Attribute number supported by old backends
 */

typedef enum {
	_DtCm_old_attr_unknown,
	_DtCm_old_attr_id,
	_DtCm_old_attr_time,
	_DtCm_old_attr_type,
	_DtCm_old_attr_type2,
	_DtCm_old_attr_duration,
	_DtCm_old_attr_what,
	_DtCm_old_attr_author,
	_DtCm_old_attr_beep_reminder,
	_DtCm_old_attr_flash_reminder,
	_DtCm_old_attr_mail_reminder,
	_DtCm_old_attr_popup_reminder,
	_DtCm_old_attr_repeat_type,
	_DtCm_old_attr_repeat_times,
	_DtCm_old_attr_showtime,
	_DtCm_old_attr_status,
	_DtCm_old_attr_privacy,
	_DtCm_old_attr_repeat_nth_interval,
	_DtCm_old_attr_repeat_nth_weeknum,
	_DtCm_old_attr_end_date
} _DtCm_old_attrs;

/* attribute information structure */
typedef struct {
	int		index;
	CSA_enum	type;
	int		fst_vers;       /* 1st data version supporting this
					 * attr
					 */
	_DtCm_old_attrs	oldattr;
	boolean_t	nex_ro;		/* readonly for non-extensible file 
					 * version
					 */
	boolean_t	ex_ro;		/* readonly for extensible file version
					 */
} _DtCmAttrInfo;

/* calendar attribute information */
extern char *_CSA_CALENDAR_ATTRIBUTE_NAMES[];
extern _DtCmAttrInfo _CSA_cal_attr_info[];

/* entry attribute information */
extern char *_CSA_ENTRY_ATTRIBUTE_NAMES[];
extern _DtCmAttrInfo _CSA_entry_attr_info[];

/*
 * external function declarations
 */

extern CSA_return_code _DtCm_check_cal_csa_attributes(int	 fvers,
						      CSA_uint32 num_attrs,
						      CSA_attribute	*attrs,
						      char		*cname,
						      boolean_t	checkreadonly,
						      boolean_t	firsttime,
						      boolean_t	checkattrnum);

extern CSA_return_code _DtCm_check_cal_cms_attributes(int		fvers,
						      CSA_uint32	num_attrs,
						      cms_attribute	*attrs,
						      char		*owner,
						      char		*cname,
						      boolean_t	checkreadonly,
						      boolean_t	firsttime,
						      boolean_t	checkattrnum);

extern CSA_return_code	_DtCm_check_entry_attributes(int	fversion,
						     CSA_uint32	size,
						     CSA_attribute	*attrs,
						     CSA_flags	utype,
						     boolean_t	checkattrnum);

extern CSA_return_code _DtCm_check_entry_cms_attributes(int		fvers,
							CSA_uint32	num_attrs,
							cms_attribute	*attrs,
							CSA_flags	utype,
							boolean_t	checkattrnum);

extern CSA_return_code	_DtCm_copy_cms_attributes(CSA_uint32 srcsize,
						  cms_attribute *srcattrs,
						  CSA_uint32 *dstsize,
						  cms_attribute **dstattrs);

extern CSA_return_code	_DtCm_copy_cms_attribute(cms_attribute	*to,
						 cms_attribute	*from,
						 boolean_t copyname);

extern CSA_return_code	_DtCm_copy_cms_attr_val(cms_attribute_value *from,
						cms_attribute_value **to);

extern cms_access_entry *_DtCm_copy_cms_access_list(cms_access_entry *alist);

extern CSA_date_time_list _DtCm_copy_date_time_list(CSA_date_time_list dlist);

extern CSA_return_code _DtCm_cms2csa_attribute(cms_attribute from,
					       CSA_attribute *to);

extern CSA_return_code _DtCm_cms2csa_attrval(cms_attribute_value *from,
					     CSA_attribute_value **to);

extern CSA_return_code _DtCm_cms2csa_access_list(cms_access_entry *cmslist,
						 CSA_access_rights **csalist);

extern CSA_return_code _DtCm_csa2cms_access_list(CSA_access_rights *csalist,
						 cms_access_entry **cmslist);

extern CSA_return_code _DtCm_copy_reminder(CSA_reminder *from,
					   CSA_reminder **to);

extern CSA_return_code _DtCm_copy_opaque_data(CSA_opaque_data *from,
					      CSA_opaque_data **to);

extern void		_DtCm_free_csa_access_list(CSA_access_list alist);

extern void 		_DtCm_free_cms_attributes(CSA_uint32 size,
						  cms_attribute *attrs);

extern void 		_DtCm_free_attributes(CSA_uint32 size,
					      CSA_attribute * attrs);

extern void		_DtCm_free_cms_attribute_values(CSA_uint32 size,
							cms_attribute *attrs);

extern void		_DtCm_free_attribute_values(CSA_uint32 size,
						    CSA_attribute *attrs);

extern void		_DtCm_free_cms_attribute_value(cms_attribute_value *val);

extern void		_DtCm_free_attribute_value(CSA_attribute_value *val);

extern void		_DtCm_free_cms_access_entry(cms_access_entry *list);

extern void		_DtCm_free_date_time_list(CSA_date_time_list list);

extern void		_DtCm_free_reminder(CSA_reminder *val);

extern void		_DtCm_free_opaque_data(CSA_opaque_data *val);

extern char 		*_DtCm_old_reminder_name_to_name(char *oldname);

extern int		_DtCm_old_reminder_name_to_index(char *name);

extern CSA_return_code	_DtCm_get_old_attr_by_name(char *name,
						   _DtCm_old_attrs *attr);

extern CSA_return_code	_DtCm_get_old_attr_by_index(int index,
						    _DtCm_old_attrs *attr);

extern CSA_return_code	_DtCm_set_uint32_attrval(CSA_uint32 numval,
						 cms_attribute_value **val);

extern CSA_return_code	_DtCm_set_sint32_attrval(CSA_sint32 numval,
						 cms_attribute_value **val);

extern CSA_return_code	_DtCm_set_string_attrval(char *strval,
						 cms_attribute_value **val,
						 CSA_enum type);

extern CSA_return_code	_DtCm_set_user_attrval(char *user,
					       cms_attribute_value **val);

extern CSA_return_code	_DtCm_set_reminder_attrval(CSA_reminder *remval,
						   cms_attribute_value **val);

extern CSA_return_code	_DtCm_set_access_attrval(cms_access_entry *aval,
						 cms_attribute_value **val);

extern CSA_return_code	_DtCm_set_opaque_attrval(CSA_opaque_data *opqval,
						 cms_attribute_value **val);

extern CSA_return_code	_DtCm_set_csa_access_attrval(cms_access_entry *aval,
						     CSA_attribute_value **val);

extern CSA_return_code _DtCm_set_csa_uint32_attrval(CSA_uint32 numval,
						    CSA_attribute_value **attrval);

extern CSA_return_code _DtCm_set_csa_string_attrval(char *strval,
						    CSA_attribute_value **attrval,
						    CSA_enum type);

extern void _DtCm_get_attribute_types(CSA_uint32 size, int *types);

extern CSA_return_code _DtCmUpdateAttributes(CSA_uint32	numsrc,
					     cms_attribute	*srcattrs,
					     CSA_uint32	*numdst,
					     cms_attribute	**dstattrs,
					     _DtCmNameTable	**tbl,
					     boolean_t	caltbl,
					     boolean_t	makecopy);

#endif
