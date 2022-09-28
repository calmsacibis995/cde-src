/* $XConsortium: delete.h /main/cde1_maint/1 1995/07/17 20:10:10 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _DELETE_H
#define _DELETE_H

#pragma ident "@(#)delete.h	1.5 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "cmscalendar.h"

extern CSA_return_code _DtCmsDeleteEntry(
			_DtCmsCalendar	*cal,
			char		*sender,
			uint		access,
			cms_key		*key,
			cms_entry	**entry_r);

extern CSA_return_code _DtCmsDeleteEntryAndLog(
			_DtCmsCalendar	*cal,
			char		*sender,
			uint		access,
			cms_key		*key,
			cms_entry	**entry_r);

extern CSA_return_code _DtCmsDeleteInstancesAndLog(
			_DtCmsCalendar	*cal,
			char		*sender,
			uint		access,
			cms_key		*key,
			int		scope,
			cms_entry	**newe,
			cms_entry	**olde);

#endif
