/* $XConsortium: entry.h /main/cde1_maint/2 1995/10/10 13:29:33 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _ENTRY_H
#define _ENTRY_H

#pragma ident "@(#)entry.h	1.14 97/01/31 Sun Microsystems, Inc."

/*
 * This file contains the internal data structure of entries.
 */

#include "ansi_c.h"
#include "cm.h"
#include "calendar.h"
#include "rtable4.h"

typedef struct _libentry {
	void		*handle;	/* for sanity check */
	Calendar	*cal;		/* associated calendar */
	cms_entry	*e;
	struct _libentry *next;
	struct _libentry *prev;
} _DtCm_libentry;

/* function prototypes */

extern _DtCm_libentry * _DtCm_get_libentry(CSA_entry_handle entryhandle);

extern CSA_return_code _DtCm_make_libentry(cms_entry	*cmsentry,
					   _DtCm_libentry	**entry_r);

extern CSA_return_code _DtCm_get_entry_attr_names(_DtCm_libentry *entry,
						  CSA_uint32 *num_names_r,
						  char **names_r[]);

extern CSA_return_code _DtCm_get_entry_attrs_by_name(_DtCm_libentry *entry,
						     CSA_uint32 num_names,
						     CSA_attribute_reference *names,
						     CSA_uint32 *num_attrs,
						     CSA_attribute **attrs);

extern CSA_return_code _DtCm_get_all_entry_attrs(_DtCm_libentry *entry,
						 CSA_uint32 *num_attrs,
						 CSA_attribute **attrs);

extern CSA_return_code _DtCm_libentry_to_entryh(_DtCm_libentry *elist,
						CSA_uint32 *size,
						CSA_entry_handle **entries_r);

extern CSA_return_code _DtCmCmsentriesToLibentries(_DtCmNameTable	**tbl,
						   cms_entry	*entries,
						   _DtCm_libentry	**libentries);

extern CSA_return_code _DtCm_appt4_to_libentries(char		*calname,
						 Appt_4		*appt4,
						 _DtCm_libentry	**libentries);

extern CSA_return_code _DtCm_libentries_to_appt4(_DtCm_libentry *libentries,
						 Appt_4 **appt4);

extern CSA_return_code _DtCm_reminder4_to_csareminder_detail(char *calname,
							     Reminder_4 *r4,
							     Appt_4	*appts,
							     CSA_uint32	*num_rems,
							     CSA_reminder_reference	**rems);

extern CSA_return_code _DtCm_reminder4_to_csareminder(Reminder_4 *r4,
						      CSA_uint32 *num_rems,
						      CSA_reminder_reference **rems);

extern _DtCm_libentry *_DtCm_convert_entry_wheader(_DtCm_libentry *entry);

extern void _DtCm_free_libentries(_DtCm_libentry *entries);

extern void _DtCm_free_entry_handles(CSA_uint32 num_entries,
				     CSA_entry_handle *entries);

extern void _DtCm_free_reminder_references(CSA_uint32 num_rems,
					   CSA_reminder_reference *rems);

extern void _DtCm_free_entry_content(CSA_uint32 dummy, _DtCm_libentry *entry);

extern CSA_return_code _DtCm_cms2csa_reminder_detail(Calendar	*cal,
						     cms_reminder_ref *cmsrems,
						     cms_get_entry_attr_res_item *entries,
						     CSA_uint32	*num_rems,
						     CSA_reminder_reference **csarems);

extern CSA_return_code _DtCm_convert_appt_to_reminder_detail(char *cname,
							     cms_reminder_ref	*cmsrems,
							     Appt_4 *appt,
							     CSA_uint32	*num_rems,
							     CSA_reminder_reference	**csarems);

#endif
