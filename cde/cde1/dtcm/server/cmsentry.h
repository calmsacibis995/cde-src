/* $XConsortium: cmsentry.h /main/cde1_maint/3 1995/10/03 12:30:34 barstow $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)cmsentry.h	1.8 96/11/12 Sun Microsystems, Inc."

#ifndef _CMSENTRY_H
#define _CMSENTRY_H

#include "ansi_c.h"
#include "nametbl.h"
#include "cm.h"
#include "csa.h"
#include "cmscalendar.h"

#define	_DtCM_APPOINTMENT_STRING	"Appointment"
#define	_DtCM_TODO_STRING		"To Do"
#define	_DtCM_MEMO_STRING		"Memo"
#define	_DtCM_ENTRY_STRING		"Calendar Entry"

extern CSA_return_code _DtCmsMakeHashedEntry(
			_DtCmsCalendar	*cal,
			CSA_uint32	num,
			cms_attribute	*attrs,
			cms_entry	**entry);

extern void _DtCmsFreeEntryAttrResItem(cms_get_entry_attr_res_item *elist);

extern CSA_return_code _DtCmsGetCmsEntryForClient(
			cms_entry	*e,
			cms_entry	**e_r,
			boolean_t	timeonly);

#endif
