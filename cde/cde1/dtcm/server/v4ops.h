/* $XConsortium: v4ops.h /main/cde1_maint/3 1995/10/10 13:37:06 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _V4OPS_H
#define _V4OPS_H

#pragma ident "@(#)v4ops.h	1.13 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "rtable4.h"
#include "cm.h"
#include "log.h"
#include "cmscalendar.h"

#define is_appointment(p_appt)  	((p_appt)->period.period == single_4)
#define is_repeater(p_appt)     	((p_appt)->period.period != single_4)

typedef struct __DtCmsEntryId {
	long id;
	struct __DtCmsEntryId *next;
} _DtCmsEntryId;

extern CSA_return_code _DtCmsInsertAppt(_DtCmsCalendar *cal, Appt_4 *appt4);

extern CSA_return_code _DtCmsDeleteAppt(
			_DtCmsCalendar	*cal,
			char		*user,
			uint		access,
			Id_4		*p_key,
			Appt_4		**appt_r);

extern CSA_return_code _DtCmsDeleteApptAndLog(
			_DtCmsCalendar	*cal,
			char		*user,
			uint		access,
			Id_4		*key,
			Appt_4		**oldappt);

extern CSA_return_code _DtCmsDeleteApptInstancesAndLog(
			_DtCmsCalendar	*cal,
			char		*source,
			uint		access,
			Id_4		*key,
			Options_4	option,
			int		*remain,
			Appt_4		**oldappt);

extern CSA_return_code _DtCmsChangeAll(
			_DtCmsCalendar	*cal,
			char		*source,
			uint		access,
			Id_4		*p_key,
			Appt_4		*newa,
			Appt_4		**oldappt);

extern CSA_return_code _DtCmsChangeSome(
			_DtCmsCalendar	*cal,
			char		*source,
			uint		access,
			Id_4		*p_key,
			Appt_4		*p_appt,
			Options_4	option,
			Appt_4		**oldappt);

extern CSA_return_code _DtCmsInsertApptAndLog(
			_DtCmsCalendar	*cal,
			Appt_4		*appt);

extern _DtCmsComparisonResult _DtCmsCompareAppt(Id_4 *key, caddr_t data);

extern _DtCmsComparisonResult _DtCmsCompareRptAppt(Id_4 *key, caddr_t data);

extern caddr_t _DtCmsGetApptKey(caddr_t data);

extern CSA_return_code v4_transact_log(
			char		*calendar,
			Appt_4		*a,
			_DtCmsLogOps	op);

extern CSA_return_code _DtCmsLookupRangeV4(
			_DtCmsCalendar	*cal,
			char		*user,
			uint		access,
			Range_4		*p_range,
			boolean_t	no_end_time_range,
			long		end1,
			long		end2,
			boolean_t	(*match_func)(),
			uint		num_attrs,
			cms_attribute	*attrs,
			CSA_enum	*ops,
			Appt_4		**appt_r,
			Abb_Appt_4	**abbr_r);

extern CSA_return_code _DtCmsLookupKeyrangeV4(
			_DtCmsCalendar	*cal,
			char		*user,
			uint		access,
			boolean_t	no_start_time_range,
			boolean_t	no_end_time_range,
			time_t		start1,
			time_t		start2,
			time_t		end1,
			time_t		end2,
			long		id,
			boolean_t	(*match_func)(),
			uint		num_attrs,
			cms_attribute	*attrs,
			CSA_enum	*ops,
			Appt_4		**appt_r,
			Abb_Appt_4	**abbr_r);

extern CSA_return_code _AddToLinkedAppts(
			Appt_4	*p_appt,
			char	*user,
			uint	access,
			caddr_t	*ilp);

extern CSA_return_code _AddToLinkedAbbrAppts(
			Appt_4	*p_appt,
			char	*user,
			uint	access,
			caddr_t	*ilp);

extern Privacy_Level_4 _GetAccessLevel(
			char	*user,
			uint	access,
			Appt_4	*p_appt);

extern Appt_4 * _AddApptInOrder(Appt_4 * head, Appt_4 * aptr);

extern Abb_Appt_4 * _AddAbbApptInOrder(Abb_Appt_4 *head, Abb_Appt_4 *aptr);

extern CSA_return_code _DtCmsSetV4AccessListAndLog(
			_DtCmsCalendar	*cal,
			Access_Entry_4	*alist);

extern CSA_return_code _DtCmsGetV4Reminders(
			_DtCmsCalendar	*cal,
			long		tick,
			Reminder_4	**rem_r,
			_DtCmsEntryId	**ids_r);

extern void _DtCmsFreeEntryIds(_DtCmsEntryId *ids);

extern CSA_return_code _DtCmsTruncateElist(
			Appt_4		*parent_p,
			int		remain,
			Except_4	**excpt);

extern CSA_return_code _DtCmsSetV4AccessListInCal(
			_DtCmsCalendar *cal,
			Access_Entry_4 *e);

extern CSA_return_code _DtCmsCopyAccessList4(
			Access_Entry_4 *alist,
			Access_Entry_4 **newlist);

#endif
