/* $XConsortium: v5ops.h /main/cde1_maint/2 1995/09/06 08:30:18 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _V5OPS_H
#define _V5OPS_H

#pragma ident "@(#)v5ops.h	1.13 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "cmscalendar.h"
#include "rerule.h"
#include "repeat.h"

extern _DtCmsComparisonResult _DtCmsCompareEntry(
				cms_key *key,
				caddr_t data);

extern _DtCmsComparisonResult _DtCmsCompareRptEntry(
				cms_key *key,
				caddr_t data);

extern caddr_t _DtCmsGetEntryKey(caddr_t data);

extern CSA_return_code _DtCmsSetLastUpdate(cms_entry *entry);

extern void _DtCmsConvertToOnetime(cms_entry *entry, RepeatEvent *re);

extern int _DtCmsGetDuration(cms_entry *eptr);

extern CSA_return_code _DtCmsCheckInitialAttributes(cms_entry *entry);

extern CSA_return_code _DtCmsCheckStartEndTime(cms_entry *entry);

extern void _DtCmsCleanupExceptionDates(cms_entry *newe, long ftick);

extern int _DtCmsNumberExceptionDates(cms_entry *entry);

extern CSA_return_code _DtCmsUpdateDurationInRule(
				cms_entry	*entry,
				uint		remain);

extern CSA_return_code _DtCmsAddEndDateToRule(
				cms_attribute	*attr,
				RepeatEvent	*re,
				long		time);

extern CSA_return_code _DtCmsUpdateAttributes(
			CSA_uint32	numsrc,
			cms_attribute	*srcattrs,
			CSA_uint32	*numdst,
			cms_attribute	**dstattrs,
			_DtCmNameTable	**tbl,
			boolean_t	caltbl,
			int		**types);

#endif
