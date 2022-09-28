/* $XConsortium: convert5-4.h /main/cde1_maint/1 1995/07/17 19:55:32 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CONVERT5_4_H
#define _CONVERT5_4_H

#pragma ident "@(#)convert5-4.h	1.11 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "rtable4.h"

/*
 * conversion routines for rpc.cmsd version 5 to rpc.cmsd version 4 data types
 */
extern CSA_return_code _DtCm_cmsattrs_to_apptdata(
				uint num_attrs,
				cms_attribute *attrs,
				Appt_4 *appt4);

extern CSA_return_code _DtCm_attrs_to_apptdata(
				uint size,
				CSA_attribute *attrs,
				Appt_4 *appt);

extern CSA_return_code _DtCm_cms_entry_to_appt4(
				cms_entry *entry,
				Appt_4 **appt4);

extern CSA_return_code _DtCm_attrs_to_appt4(
				uint num_attrs,
				CSA_attribute *attrs,
				Appt_4 **appt4_r);

extern CSA_return_code _DtCm_scope_to_options4(
				CSA_enum scope,
				Options_4 *opt);

extern CSA_return_code _DtCm_csaaccesslist_toaccessentry4(
				CSA_access_list alist,
				Access_Entry_4 **a4);

extern int _DtCm_string_to_eventtype4(
				char *val,
				Event_Type_4 *tag);

extern int _DtCm_rtype_to_interval4(
				int val,
				Interval_4 *period);

extern int _DtCm_status_to_apptstatus4(
				int val,
				Appt_Status_4 *astat);

extern int _DtCm_classification_to_privacy4(
				int val,
				Privacy_Level_4 *privacy);

extern CSA_return_code _DtCm_attrs_to_eventtype4(
				cms_attribute_value	*type,
				cms_attribute_value	*stype,
				Tag_4			*tag);

extern CSA_return_code _DtCm_remove_reminder(char *rem, Appt_4 *appt);

extern CSA_return_code _DtCm_add_reminder(
				char *rem,
				CSA_reminder * val,
				Appt_4 *appt);

extern int _DtCmAccessRightToV4AccessType(unsigned int access);

extern CSA_return_code _DtCm_cmskey_to_uid4(
				int	num,
				cms_key	*cmskeys,
				Uid_4	**uids);

extern void _DtCm_free_uid4(Uid_4 *uids);

#endif
