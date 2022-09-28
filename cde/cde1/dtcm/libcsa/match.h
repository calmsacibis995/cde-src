/* $XConsortium: match.h /main/cde1_maint/3 1995/10/10 13:30:20 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _MATCH_H
#define _MATCH_H

#pragma ident "@(#)match.h	1.10 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "rtable4.h"

extern CSA_return_code _DtCmHashCriteria(_DtCmNameTable	*tbl,
					 CSA_uint32	num_attrs,
					 CSA_attribute	*csaattrs,
					 cms_attribute	*cmsattrs,
					 CSA_enum	*ops,
					 boolean_t	*no_match,
					 boolean_t	*no_start_time_range,
					 boolean_t	*no_end_time_range,
					 time_t		*start1,
					 time_t		*start2,
					 time_t		*end1,
					 time_t		*end2,
					 long		*id,
					 CSA_uint32	*hnum,
					 cms_attribute	**hattrs,
					 CSA_enum	**hops);

extern void _DtCmFreeHashedArrays(CSA_uint32	hnum,
				  cms_attribute	*hattrs,
				  CSA_enum	*hops);

extern Appt_4 *_DtCm_match_appts(Appt_4		*appts,
				 long		id,
				 boolean_t	no_end_time_range,
				 time_t		end1,
				 time_t		end2,
				 CSA_uint32	num_attrs,
				 cms_attribute	*attrs,
				 CSA_enum	*ops);

extern boolean_t _DtCm_match_one_appt(Appt_4 *appt,
				      uint num_attrs,
				      cms_attribute * attrs,
				      CSA_enum *ops);

extern Reminder_4 *_DtCm_match_reminders(Reminder_4 *rems,
					 uint num_names,
					 char **names);

extern CSA_return_code _DtCm_check_operator(uint size,
					    CSA_attribute *csaattrs,
					    cms_attribute *cmsattrs,
					    CSA_enum *ops);

extern boolean_t _DtCm_match_sint32_attribute(cms_attribute_value *val1,
					      cms_attribute_value *val2,
					      CSA_enum op);

extern boolean_t _DtCm_match_uint32_attribute(cms_attribute_value *val1,
					      cms_attribute_value *val2,
					      CSA_enum op);

extern boolean_t _DtCm_match_time_attribute(cms_attribute_value *val1,
					    cms_attribute_value *val2,
					    CSA_enum op);

extern boolean_t _DtCm_match_time_duration_attribute(cms_attribute_value *val1,
						     cms_attribute_value *val2,
						     CSA_enum op);

extern boolean_t _DtCm_match_string_attribute(cms_attribute_value *val1,
					      cms_attribute_value *val2,
					      CSA_enum op);

extern boolean_t _DtCm_match_reminder_attribute(cms_attribute_value *val1,
						cms_attribute_value *val2,
						CSA_enum op);

#endif
