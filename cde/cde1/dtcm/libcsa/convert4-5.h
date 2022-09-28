/* $XConsortium: convert4-5.h /main/cde1_maint/1 1995/07/17 19:54:53 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CONVERT4_5_H
#define _CONVERT4_5_H

#pragma ident "@(#)convert4-5.h	1.13 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "rtable4.h"

/*
 * functions to convert v4 data structures
 */
extern CSA_return_code _DtCm_accessentry4_to_cmsaccesslist(
				Access_Entry_4 *a4,
				cms_access_entry **alist);

extern CSA_return_code _DtCm_appt4_to_attrs(
				char		*calname,
				Appt_4		*a4,
				uint		num_attrs,
				cms_attribute	*attrs,
				boolean_t	rerule);

extern int _DtCm_eventtype4_to_type(Event_Type_4 tag);

extern char *_DtCm_eventtype4_to_subtype(Event_Type_4 tag);

extern CSA_sint32 _DtCm_interval4_to_rtype(Interval_4 val);

extern CSA_sint32 _DtCm_apptstatus4_to_status(Appt_Status_4 stat);

extern CSA_sint32 _DtCm_privacy4_to_classification(Privacy_Level_4 p);

extern CSA_return_code _DtCmAttr4ToReminderValue(
				Attribute_4		*remval,
				cms_attribute_value	**attrval);

#endif
