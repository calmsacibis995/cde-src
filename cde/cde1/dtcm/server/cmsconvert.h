/* $XConsortium: cmsconvert.h /main/cde1_maint/2 1995/09/06 08:26:36 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CMSCONVERT_H
#define _CMSCONVERT_H

#pragma ident "@(#)cmsconvert.h	1.9 97/05/21 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "rtable4.h"
#include "v4ops.h"

extern CSA_return_code _DtCmsCmsentriesToAppt4ForClient(
			cms_entry	*entries,
			Appt_4		**appt);

extern CSA_return_code _DtCmsCmsentriesToAbbrAppt4ForClient(
			cms_entry	*entries,
			Abb_Appt_4	**abbr);

extern CSA_return_code _DtCmsCmsentryToAbbrAppt4(
			cms_entry	*entry,
			Abb_Appt_4	**abbr);

extern Access_Entry_4 *_DtCmsConvertV5AccessList(
			cms_access_entry	*cmslist,
			boolean_t		strictmode);

extern cms_access_entry *_DtCmsConvertV4AccessList(
			Access_Entry_4 *alist, char *name);

extern CSA_return_code _DtCmsReminderRefToReminder(
			cms_reminder_ref	*rems,
			Reminder_4		**r4);

extern CSA_return_code _DtCmsV4ReminderToReminderRef(
			char			*calname,
			Reminder_4		*r4,
			_DtCmsEntryId		*ids,
			cms_reminder_ref	**rems);

extern CSA_return_code _DtCmsCmsAccessToV4Access(
				cms_access_entry *alist,
				Access_Entry_4 **a4);

extern CSA_return_code _DtCmsAppt4ToCmsentriesForClient(
			char		*calname,
			Appt_4		*appt,
			cms_entry	**e_r);

extern CSA_return_code _DtCmsAppt4ToCmsentry(
			char		*calname,
			Appt_4		*a4,
			cms_entry	**entry_r,
			boolean_t	rerule);

extern CSA_return_code _DtCmsAttrsToAppt4(
			CSA_uint32	size,
			cms_attribute	*attrs,
			Appt_4		*appt,
			boolean_t	check);

#endif
