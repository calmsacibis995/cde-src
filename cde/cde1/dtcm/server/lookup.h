/* $XConsortium: lookup.h /main/cde1_maint/3 1995/10/10 13:34:28 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)lookup.h	1.7 96/11/12 Sun Microsystems, Inc."

#ifndef _LOOKUP_H
#define _LOOKUP_H

#include "ansi_c.h"
#include "cm.h"
#include "cmscalendar.h"

extern _DtCmsComparisonResult _DtCmsCompareEntry(
				cms_key *key,
				caddr_t data);

extern _DtCmsComparisonResult _DtCmsCompareRptEntry(
				cms_key *key,
				caddr_t data);

extern caddr_t _DtCmsGetEntryKey(caddr_t data);

extern CSA_return_code _DtCmsLookupEntries(
			_DtCmsCalendar	*cal,
			char		*sender,
			CSA_flags	access,
			time_t		start1,
			time_t		start2,
			boolean_t	no_end_time_range,
			time_t		end1,
			time_t		end2,
			CSA_uint32	num_attrs,
			cms_attribute	*attrs,
			CSA_enum	*ops,
			cms_entry	**entries);

extern CSA_return_code _DtCmsLookupEntriesById(
			_DtCmsCalendar	*cal,
			char		*sender,
			CSA_flags	access,
			boolean_t	no_start_time_range,
			boolean_t	no_end_time_range,
			time_t		start1,
			time_t		start2,
			time_t		end1,
			time_t		end2,
			long		id,
			CSA_uint32	num_attrs,
			cms_attribute	*attrs,
			CSA_enum	*ops,
			cms_entry	**entries);

extern CSA_return_code _DtCmsEnumerateSequenceById(
			_DtCmsCalendar	*cal,
			char		*sender,
			CSA_flags	access,
			boolean_t	no_start_time_range,
			boolean_t	no_end_time_range,
			time_t		start1,
			time_t		start2,
			time_t		end1,
			time_t		end2,
			long		id,
			CSA_uint32	num_attrs,
			cms_attribute	*attrs,
			CSA_enum	*ops,
			cms_entry	**entries);

extern CSA_return_code _DtCmsLookupEntriesByKey(
			_DtCmsCalendar	*cal,
			char		*sender,
			CSA_flags	access,
			CSA_uint32	num_keys,
			cms_key		*keys,
			CSA_uint32	num_names,
			cms_attr_name	*names,
			cms_get_entry_attr_res_item **res);


extern CSA_return_code _DtCmsGetEntryAttrByKey(
			_DtCmsCalendar	*cal,
			char		*sender,
			CSA_flags	access,
			cms_key		key,
			CSA_uint32	num_names,
			cms_attr_name	*names,
			cms_entry	**entry_r,
			cms_get_entry_attr_res_item **res_r);

#endif
