/* $XConsortium: access.h /main/cde1_maint/1 1995/07/17 20:05:54 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _ACCESS_H
#define _ACCESS_H

#pragma ident "@(#)access.h	1.19 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "rtable4.h"
#include "cmscalendar.h"

#define _DTCMS_SUPER_USER	"root"

#define _DTCMS_HAS_VIEW_CALENDAR_ATTR_ACCESS(a) \
		((a) & (CSA_VIEW_CALENDAR_ATTRIBUTES | CSA_OWNER_RIGHTS))

#define _DTCMS_HAS_INSERT_CALENDAR_ATTR_ACCESS(a) \
		((a) & (CSA_INSERT_CALENDAR_ATTRIBUTES | CSA_OWNER_RIGHTS))

#define _DTCMS_HAS_CHANGE_CALENDAR_ATTR_ACCESS(a) \
		((a) & (CSA_CHANGE_CALENDAR_ATTRIBUTES | CSA_OWNER_RIGHTS))

#define _DTCMS_HAS_VIEW_ACCESS(a) ((a) & (CSA_VIEW_PUBLIC_ENTRIES | \
					    CSA_VIEW_CONFIDENTIAL_ENTRIES | \
					    CSA_VIEW_PRIVATE_ENTRIES | \
					    CSA_ORGANIZER_RIGHTS | \
					    CSA_SPONSOR_RIGHTS | \
					    CSA_OWNER_RIGHTS))

#define _DTCMS_HAS_INSERT_ACCESS(a) ((a) & (CSA_INSERT_PUBLIC_ENTRIES | \
					    CSA_INSERT_CONFIDENTIAL_ENTRIES | \
					    CSA_INSERT_PRIVATE_ENTRIES | \
					    CSA_OWNER_RIGHTS))

#define _DTCMS_HAS_CHANGE_ACCESS(a) ((a) & (CSA_CHANGE_PUBLIC_ENTRIES | \
					    CSA_CHANGE_CONFIDENTIAL_ENTRIES | \
					    CSA_CHANGE_PRIVATE_ENTRIES | \
					    CSA_ORGANIZER_RIGHTS | \
					    CSA_SPONSOR_RIGHTS | \
					    CSA_OWNER_RIGHTS))

#define _DTCMS_HAS_V4_BROWSE_ACCESS(a) \
		((a) & (access_read_4 | CSA_OWNER_RIGHTS))

#define _DTCMS_HAS_V4_WRITE_ACCESS(a) \
		((a) & (access_write_4 | CSA_OWNER_RIGHTS))

typedef struct __DtCmsSender {
	char		*name;
	boolean_t	knownuser;
	boolean_t	localroot;
} _DtCmsSender;

extern CSA_return_code _DtCmsLoadAndCheckAccess(
			struct svc_req	*svcrq,
			char		*target,
			_DtCmsSender	**sender,
			CSA_flags	*access,
			_DtCmsCalendar	**cal);

extern CSA_return_code _DtCmsGetClientInfo(
			struct svc_req	*svcrq,
			_DtCmsSender	**sender);

extern boolean_t _DtCmsIsFileOwner(char *owner, char *user, char *target);

extern void _DtCmsShowAccessList(Access_Entry_4 *l);

extern Access_Entry_4 *_DtCmsCalendarAccessList(_DtCmsCalendar *cal);

extern CSA_return_code _DtCmsCheckViewAccess(
			char *user,
			CSA_flags access,
			cms_entry *eptr,
			boolean_t *timeonly);

extern CSA_return_code _DtCmsCheckChangeAccess(
			char		*user,
			CSA_flags	access,
			cms_entry	*eptr);

extern CSA_flags _DtCmsClassToInsertAccess(cms_entry *entry);

extern CSA_flags _DtCmsClassToViewAccess(cms_entry *entry);

extern CSA_flags _DtCmsClassToChangeAccess(cms_entry *entry);

extern CSA_return_code _DtCmsSetAccessControlList(
			char		*value,
			boolean_t	setTrusted);

extern CSA_return_code _DtCmsCheckCreateAccess(_DtCmsSender *user);

extern void _DtCmsUnsetAccessControlList();

#endif
