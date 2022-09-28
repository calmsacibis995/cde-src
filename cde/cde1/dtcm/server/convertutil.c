/* $XConsortium: convertutil.c /main/cde1_maint/2 1995/11/14 17:01:34 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)convertutil.c	1.20 97/04/01 Sun Microsystems, Inc."

/*
 * This program does one of the followings depending on the option specified:
 * - convert version 3 to version 4 calendar data (may trim the calendar)
 * - convert version 4 to version 3 calendar data (may trim the calendar)
 * - trim a calendar file
 * - do sanity check on the calendar
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <rpc/rpc.h>
#include <values.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <locale.h>
#include <LocaleXlate.h>
#include <nl_types.h>
#include "csa.h"
#include "rtable4.h"
#include "appt4.h"
#include "cmscalendar.h"
#include "convert4-5.h"
#include "attr.h"
#include "v5ops.h"
#include "log.h"
#include "cmsdata.h"
#include "insert.h"
#include "cmsconvert.h"
#include "updateattrs.h"
#include "misc.h"
#include "iso8601.h"
#include "lutil.h"
#include "garbage.h"
#include "reutil.h"
#include "repeat.h"

/* routine to get common locale/charset name */
extern void _DtI18NXlateOpToStdLocale(
     		char       *operation,
     		char       *opLocale,
     		char       **ret_stdLocale,
     		char       **ret_stdLang,
     		char       **ret_stdSet);

#define	_V4OVERHEAD		3500

#define SDTCM_CONVERT_CAT	"sdtcm_convert"	/* message catalog */

#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE	0
#else
#if defined(sun) && (_XOPEN_VERSION == 3)
#undef NL_CAT_LOCALE
#define NL_CAT_LOCALE	0
#endif
#endif

/* return values used in the file */
#define _OK			0
#define _CANT_ACCESS		-1
#define _NO_SPACE		-2
#define _FAILURE		-3
#define _REJECT_CORRECTION	-4
#define	_CORRECTION_DONE	-5
#define _CANT_CHANGE_MODE	-6

/* global variables */
boolean_t strict_csa_model = _B_FALSE;	/* needed in access.c */
char	*pgname;
int	debug;
int	daemon_gid, daemon_uid, g_owneruid, g_grpuid, g_owneruid_old;
int	g_useruid = -1;			/* uid derived from the cal name */
int	g_uid;				/* real user id of the process */
mode_t	g_filemode;
_DtCmsCalendar *g_newcal;	        /* new calendar */
char	g_target[BUFSIZ] = "";		/* calendar@host */
char	g_calname[BUFSIZ] = "";
char	g_calname_old[BUFSIZ] = "";
char	g_owner[BUFSIZ] = "";		/* owner of the calendar */
char	g_charsetargv[BUFSIZ] = "";	/* character set */
char	g_bdirargv[BUFSIZ] = "";	/* backup directory */
long	g_startdate = -1;		/* start date to convert appointment */
int	g_output_ver = -1;		/* output version of new calendar */
int	g_total = 0;			/* total number of appt in old cal */
int	g_onetime = 0;			/* number of one-time appt in old cal */
int	g_repeating = 0;		/* number of repeating appt in old cal
					 */
int	g_converted_onetime = 0;	/* number of one-time appt converted */
int	g_converted_repeating = 0;	/* number of repeating appt converted */
int	g_failed = 0;			/* number of appt failed to be converted
					 */
long	g_now;				/* the current time */
nl_catd	g_catd;				/* message catalog description */
boolean_t g_sanity_check = _B_FALSE;	/* if _B_TRUE, just do sanity check on
					 * cal
					 */
boolean_t g_do_copy = _B_FALSE;		/* no need to copy if -d option is
					 * not specified
					 */

/* macro to switch euid to real user id */
#define _SWITCH_TO_REAL_USER	{if (g_uid != 0) seteuid(g_uid);}

/* macro to switch euid to root id */
#define _SWITCH_TO_SUPER_USER	{if (g_uid != 0) seteuid(0);}

#ifdef __osf__
u_int g_svr_version = 0;
#else
u_long	g_svr_version = 0;
#endif

boolean_t g_fixit = _B_FALSE;

static void
_PrintUsage()
{
	/*
	 * NL_COMMENT
	 * Attention Translator:
	 * messages number 1 to 5 are to be print out all at the same time
	 */
	fprintf(stderr, catgets(g_catd, 1, 1, "usage: %s [-v output-version] [-s <mm/dd/yy>] [-c character-set] [-d backup-directory] calendar\n"), pgname);
	fprintf(stderr, catgets(g_catd, 1, 2, "\t-v specifies the output version, 3 or 4\n"));
	fprintf(stderr, catgets(g_catd, 1, 3, "\t-s specifies the start date to do the conversion\n"));
	fprintf(stderr, catgets(g_catd, 1, 4, "\t-c specifies the locale name to be used in the character set attribute\n"));
	fprintf(stderr, catgets(g_catd, 1, 5, "\t-d specifies the backup-directory\n"));
}

/*
 * convert specified startdate ("mm/dd/yy") to a tick
 */
static int
_get_startdate(char *sdatestr)
{
	int	m, d, y, i;
	char	*ptr;
	char	buf[10];
	struct	tm tm;

	i = 0;
	ptr = buf;
	while (*sdatestr) {

		if (isdigit(*sdatestr)) {
			*ptr++ = *sdatestr++;
			continue;
		} else if (*sdatestr == '/')
			sdatestr++;
		else
			return (_FAILURE);

		*ptr = NULL;
		switch (i) {
		case 0: /* get month */
			m = atoi(buf);
			if (m < 1 || m > 12)
				return (_FAILURE); /* bad month */
			break;
		case 1: /* get day */
			d = atoi(buf);
			if (d < 1 || d > 31)
				return (_FAILURE); /* bad day */
			break;
		default:
			return (_FAILURE);
		}
		i++;
		ptr = buf;
	}

	if (i != 2) /* || *sdatestr != NULL) */
		return (_FAILURE);
	else {
		/* get year */
		*ptr = NULL;
		y = atoi(buf);
		if (y < 70)
			y += 2000;
		else if (y < 100)
			y += 1900;

		if (y < 1970 || y > 2037)
			return (_FAILURE); /* bad year */
	}

	memset((void *)&tm, NULL, sizeof(struct tm));
	tm.tm_mday = d;
	tm.tm_mon = m - 1;
	tm.tm_year = y - 1900;
	tm.tm_isdst = -1;
	g_startdate = mktime(&tm);
	return (_OK);
}

/*
 * extract the command line options
 */
static int
get_args(int argc, char **argv)
{
	int	opt;

	while ((opt = getopt(argc, argv, "v:s:c:d:")) != -1) {
		switch (opt) {
		case 'v': /* output version */
			g_output_ver = atoi(optarg);
			if (g_output_ver != 3 && g_output_ver != 4) {
				fprintf(stderr, catgets(g_catd, 1, 6,
					"Invalid version number specified.\n"));
				goto error;
			}
			break;
		case 's':
			if (_get_startdate(optarg)) {
				fprintf(stderr, catgets(g_catd, 1, 7,
					"Invalid start date specified.\n"));
				goto error; 
			}
			break;
		case 'c':
			strcpy(g_charsetargv, optarg);
			break;
		case 'd':
			strcpy(g_bdirargv, optarg);
			break;
		case '?':
			goto error;
		}
	}

	if (optind == argc) {
		/*
		 * NL_COMMENT
		 * Attention Translator:
		 * If message number 58 is printed, it will be
		 * followed by messages number 1 to 5.
		 */
		fprintf(stderr, catgets(g_catd, 1, 58,
			"Please specify a calendar.\n"));
	} else if ((argc - optind) == 1) {
		strcpy(g_calname, argv[optind]);
		return (_OK);
	}
error:
	_PrintUsage();
	return (_FAILURE);
}

/*
 * converts a list of exception dates to a list of ordinal numbers
 */
static CSA_return_code
_ExceptionDateToList(List_node *lnode, cms_entry *entry, Except_4 **elist)
{
	CSA_return_code		stat = CSA_SUCCESS;
	Except_4		*head, *eptr;
	CSA_date_time_entry	*dptr;
	int			ordinal;
	time_t			tick, etick;
	RepeatEventState	*restate;

	if (entry->attrs[CSA_ENTRY_ATTR_EXCEPTION_DATES_I].value == NULL ||
	    entry->attrs[CSA_ENTRY_ATTR_EXCEPTION_DATES_I].value->item.\
	    date_time_list_value == NULL) {
		*elist = NULL;
		return (CSA_SUCCESS);
	} else
		dptr = entry->attrs[CSA_ENTRY_ATTR_EXCEPTION_DATES_I].value->\
			item.date_time_list_value;

	tick = ClosestTick(entry->key.time, entry->key.time, lnode->re,
		&restate);
	for (ordinal = 1, head = NULL; dptr != NULL; dptr = dptr->next) {
		_csa_iso8601_to_tick(dptr->date_time, &etick);
		while (tick > 0 && tick < etick) {
			ordinal++;
			tick = NextTick(tick, entry->key.time, lnode->re,
				restate);
		}

		if ((eptr = (Except_4 *)calloc(1, sizeof(Except_4))) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		eptr->ordinal = ordinal;

		if (head != NULL)
			eptr->next = head;
		head = eptr;
	}

	if (stat == CSA_SUCCESS)
		*elist = head;
	else
		_DtCm_free_excpt4(head);

	return (stat);
}

/*
 * this is a brute force way of converting a list of ordinal numbers
 * to a list of exception dates
 */
static CSA_return_code
_ExceptionListToDate(Appt_4 *appt, cms_attribute *edateattr)
{
	CSA_return_code		stat = CSA_SUCCESS;
	Except_4		*eptr, *enext, *elast;
	cms_attribute_value	*val;
	CSA_date_time_entry	*head, *prev, *dptr;
	long			tick;
	int			count;
	char			buf[BUFSIZ];

	if ((val = (cms_attribute_value *)calloc(1,
	    sizeof(cms_attribute_value))) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);
	else
		val->type = CSA_VALUE_DATE_TIME_LIST;

	/* need to reverse the list of ordinal first */
	for (eptr = appt->exception, elast = NULL; eptr != NULL; eptr = enext) {
		enext = eptr->next;
		eptr->next = elast;
		elast = eptr;
	}
	appt->exception = elast;
	
	tick = appt->appt_id.tick;
	count = 1;
	head = NULL;
	for (eptr = appt->exception; eptr != NULL; eptr = eptr->next) {
		for (; count < eptr->ordinal; count++) {
			tick = _DtCms_next_tick_v4(tick, appt->period);
		}

		if ((dptr = (CSA_date_time_entry *)calloc(1,
		    sizeof(CSA_date_time_entry))) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if (_csa_tick_to_iso8601(tick, buf)) {
			stat = CSA_E_INVALID_DATE_TIME;
			break;
		} else if ((dptr->date_time = (char *)strdup(buf)) == NULL) {
			stat = CSA_E_INSUFFICIENT_MEMORY;
			break;
		}

		if (head == NULL)
			head = dptr;
		else
			prev->next = dptr;
		prev = dptr;
	}

	if (stat == CSA_SUCCESS) {
		val->item.date_time_list_value = head;
		edateattr->value = val;
	} else {
		_DtCm_free_date_time_list(head);
		free(edateattr->value);
	}

	return (stat);
}

/*
 * check whether we should include the appointment in the conversion.
 * Yes, if it falls on or after the start date option
 * No, if it falls before the start date option
 * For repeating appointments, retain the part that falls on or after
 * the start date option
 */
static boolean_t
_IncludeAppt(Appt_4 *appt, boolean_t *isonetime)
{
	long		tick;
	int		ordinal, ntimes, forever = 0;
	Except_4	*exceptions;

	if (appt->period.period == single_4) {
		*isonetime = _B_TRUE;
		g_onetime++;
		if (g_startdate >= 0 && appt->appt_id.tick < g_startdate)
			return (_B_FALSE);
	} else {
		*isonetime = _B_FALSE;
		g_repeating++;

                /* Old version callog file may have some repeating entries */
                /* which shows ntimes: 0.  However, these appointments do */  
                /* not show up on dtcm.  Therefore, when we do */
                /* conversion, we should not include them. */

                if (appt->ntimes == 0) {
                        return (_B_FALSE);
		} else if (g_startdate >= 0 
       			&& appt->appt_id.tick < g_startdate) {
			tick = _DtCms_last_tick_v4(appt->appt_id.tick,
				appt->period, appt->ntimes);
			if (tick < g_startdate)
				return (_B_FALSE);
			else {
				/* calculate a new startdate */
				tick = _DtCms_closest_tick_v4(g_startdate,
					appt->appt_id.tick, appt->period,
					&ordinal);

				/* calculate new ntimes */
				if (appt->ntimes == _DtCM_OLD_REPEAT_FOREVER)
					forever = 1;

				ntimes = _DtCms_get_new_ntimes_v4(
					appt->period, tick,
					_DtCms_get_ninstance_v4(appt) -
					ordinal + 1);

				/* revise the exception list */
				if (appt->exception != NULL) {
					_DtCmsTruncateElist(appt, ntimes,
						&exceptions);
					_DtCm_free_excpt4(appt->exception);
					appt->exception = exceptions;
				}
				appt->ntimes = (forever ?
					_DtCM_OLD_REPEAT_FOREVER : ntimes);
				appt->appt_id.tick = tick;
			}
		}
	}
	return (_B_TRUE);
}

/*
 * check whether we should include the entry in the conversion.
 * Yes, if it falls on or after the start date option
 * No, if it falls before the start date option
 * For repeating entries, retain the part that falls on or after
 * the start date option
 */
static boolean_t
_IncludeEntry(caddr_t node, cms_entry *entry, boolean_t *isonetime)
{
	time_t		tick, oldbod, newbod, endtime;
	int		oldntimes, ntimes, deleted;
	List_node	*lnode;
	RepeatEventState *restate;
	char		buf[BUFSIZ];
	cms_attribute	*aptr;

	if (entry->attrs[CSA_ENTRY_ATTR_RECURRENCE_RULE_I].value == NULL) {
		*isonetime = _B_TRUE;
		g_onetime++;
		if (g_startdate >= 0 && entry->key.time < g_startdate)
			return (_B_FALSE);
	} else {
		*isonetime = _B_FALSE;
		g_repeating++;
		if (g_startdate >= 0 && entry->key.time < g_startdate) {
			lnode = (List_node *)node;
			if (lnode->lasttick == 0) {
				lnode->lasttick = LastTick(entry->key.time,
							lnode->re);
			}
			if (lnode->lasttick < g_startdate)
				return (_B_FALSE);
			else {
				/* calculate a new startdate */
				tick = ClosestTick(g_startdate, entry->key.time,
					lnode->re, &restate);

				/* save old value */
				oldntimes = entry->attrs\
					[CSA_ENTRY_ATTR_NUMBER_RECURRENCES_I].\
					value->item.uint32_value;

				if (oldntimes > 25000) {
					_DtCmsUpdateDurationInRule(entry,
						CSA_X_DT_DT_REPEAT_FOREVER);
					oldntimes = CSA_X_DT_DT_REPEAT_FOREVER;
				}

				oldbod = _DtCmsBeginOfDay(entry->key.time);

				/* calculate number of instances trimmed */
				lnode->re->re_end_date = tick - 1;

				aptr = &entry->attrs[CSA_ENTRY_ATTR_EXCEPTION_DATES_I];
				deleted = CountEvents(entry->key.time,
					lnode->re, aptr->value ?
					aptr->value->item.date_time_list_value :
					NULL);

				/* clean up exception list */
				_DtCmsCleanupExceptionDates(entry, tick);

				/* calculate new ntimes */
				if (oldntimes == CSA_X_DT_DT_REPEAT_FOREVER)
					ntimes = CSA_X_DT_DT_REPEAT_FOREVER;
				else
					ntimes = oldntimes - deleted +
						_DtCmsNumberExceptionDates(entry); 

				/* restore values */
				_DtCmsUpdateDurationInRule(entry, ntimes);

				entry->key.time = tick;
				_csa_tick_to_iso8601(tick, entry->attrs\
					[CSA_ENTRY_ATTR_START_DATE_I].value->\
					item.string_value);

				_csa_tick_to_iso8601(g_now, entry->attrs\
					[CSA_ENTRY_ATTR_LAST_UPDATE_I].value->\
					item.string_value);

				/* adjust end date */
				newbod = _DtCmsBeginOfDay(entry->key.time);
				_csa_iso8601_to_tick(entry->attrs\
					[CSA_ENTRY_ATTR_END_DATE_I].value->\
					item.date_time_value, &endtime);
				endtime += (newbod - oldbod);
				_csa_tick_to_iso8601(endtime, entry->attrs\
					[CSA_ENTRY_ATTR_END_DATE_I].value->\
					item.date_time_value);
			}
		}
	}

	return (_B_TRUE);
}

static void
_PrintInsufficientMemoryMsg()
{
	fprintf(stderr, catgets(g_catd, 1, 8, "%s: insufficient memory\n"),
		pgname);
}

/*
 * return a description text for the error code
 */
static char *
_ConvertCodeToMsg(CSA_return_code error)
{
	switch (error) {
	case CSA_E_FAILURE:
		return (catgets(g_catd, 1, 9, "unknown failure"));
	case CSA_E_INSUFFICIENT_MEMORY:
		return (catgets(g_catd, 1, 10, "insufficient memory"));
	case CSA_E_INVALID_ATTRIBUTE:
		return (catgets(g_catd, 1, 11, "invalid attribute"));
	case CSA_E_INVALID_ATTRIBUTE_VALUE:
		return (catgets(g_catd, 1, 12, "invalid attribute value"));
	case CSA_E_INVALID_DATE_TIME:
		return (catgets(g_catd, 1, 13, "invalid date and time"));
	case CSA_E_INVALID_ENUM:
		return (catgets(g_catd, 1, 14, "invalid enumeration value"));
	case CSA_E_INVALID_FLAG:
		return (catgets(g_catd, 1, 15, "invalid flag"));
	case CSA_E_INVALID_RULE:
		return (catgets(g_catd, 1, 16, "invalid recurrence rule"));
	case CSA_E_UNSUPPORTED_ATTRIBUTE:
		return (catgets(g_catd, 1, 17, "unsupported attribute"));
	case CSA_E_UNSUPPORTED_ENUM:
		return (catgets(g_catd, 1, 18, "unsupported enumeration value"));
	case CSA_E_UNSUPPORTED_FLAG:
		return (catgets(g_catd, 1, 19, "unsupported flag"));
	default:
		return ("");
	}
}

/*
 * this is called when a conversion failed
 * it prints out the id and the what/summary text for the appointment
 */
static void
_PrintConversionError(long id, char *what, CSA_return_code error)
{
	char *ptr;
	char *msg;

	if (g_failed == 1)
		fprintf(stderr, catgets(g_catd, 1, 20,
			"Failed to convert appointment(s):\n"));

	if (what == NULL) what == "";

	if (ptr = strchr(what, '\n')) *ptr = NULL;
	fprintf(stderr, catgets(g_catd, 1, 21, "%d) id %d, what/summary = %s\n"),
		g_failed, id, what);
	if (ptr) *ptr = '\n';

	msg = strdup(_ConvertCodeToMsg(error));
	fprintf(stderr, catgets(g_catd, 1, 22, "error code = %d (%s)\n"),
		error, msg);
	free(msg);
}

/*
 * converts appointment data structure to an entry with the new attribute
 * format
 */
static boolean_t
_ConvertApptToEntry(caddr_t node, caddr_t a)
{
	CSA_return_code	stat;
	Appt_4		*appt = (Appt_4 *)a;
	cms_entry	*entry;
	boolean_t	wasonetime;

	g_total++;

	if (_IncludeAppt(appt, &wasonetime) == _B_FALSE)
		return (_B_FALSE);

	/* convert appt to entry and insert in new calendar */
	if (stat = _DtCmsAppt4ToCmsentry(g_target, appt, &entry, _B_TRUE)) {
		if (stat == CSA_E_INSUFFICIENT_MEMORY) {
			_PrintInsufficientMemoryMsg();
			return(_B_TRUE);
		} else {
			g_failed++;
			_PrintConversionError(appt->appt_id.key, appt->what,
				stat);
			return (_B_FALSE);
		}
	} else {
		if (appt->period.period != single_4 && appt->exception) {
			if ((stat = _ExceptionListToDate(appt,
			    &entry->attrs[CSA_ENTRY_ATTR_EXCEPTION_DATES_I]))
			    != CSA_SUCCESS) {
				if (stat == CSA_E_INSUFFICIENT_MEMORY) {
					_PrintInsufficientMemoryMsg();
					return(_B_TRUE);
				} else {
					g_failed++;
					_PrintConversionError(appt->appt_id.key,
						appt->what, stat);
					return (_B_FALSE);
				}
			}
		}

		if (_DtCmsCheckInitialAttributes(entry)) {
			_DtCm_free_cms_entry(entry);
			_PrintInsufficientMemoryMsg();
			return(_B_TRUE);
		}

		/* insert entry */
		if (stat = _DtCmsInsertEntry(g_newcal, entry)) {
			if (stat == CSA_E_INSUFFICIENT_MEMORY) {
				_PrintInsufficientMemoryMsg();
				return(_B_TRUE);
			} else {
				g_failed++;
				_DtCm_free_cms_entry(entry);
				_PrintConversionError(appt->appt_id.key,
					appt->what, stat);
				return (_B_FALSE);
			}
		}

		if (wasonetime == _B_TRUE)
			g_converted_onetime++;
		else
			g_converted_repeating++;
		return (_B_FALSE);
	}
}

/*
 * converts an entry with the new attribute format to the appointment
 * data structure
 * Data is lost since the appointment data structure is fixed
 */
static boolean_t
_ConvertEntryToAppt(caddr_t node, caddr_t e)
{
	CSA_return_code	stat;
	Appt_4		*appt;
	cms_entry	*entry = (cms_entry *)e;
	boolean_t	wasonetime;
	cms_attribute_value *aptr;

	g_total++;

	if (_IncludeEntry(node, entry, &wasonetime) == _B_FALSE)
		return (_B_FALSE);

	aptr = entry->attrs[CSA_ENTRY_ATTR_SUMMARY_I].value;

	/* convert entry to appt and insert in new calendar */
	if ((appt = _DtCm_make_appt4(_B_TRUE)) == NULL) {
		_PrintInsufficientMemoryMsg();
		return(_B_TRUE);
	}

	if (stat = _DtCmsAttrsToAppt4(entry->num_attrs + 1, entry->attrs,
	    appt, _B_FALSE)) {
		if (stat == CSA_E_INSUFFICIENT_MEMORY) {
			_PrintInsufficientMemoryMsg();
			return(_B_TRUE);
		} else {
			g_failed++;
			_PrintConversionError(entry->key.id,
				(aptr ? aptr->item.string_value : NULL), stat);
			return (_B_FALSE);
		}
	} else {
		if (wasonetime == _B_FALSE &&
		    entry->attrs[CSA_ENTRY_ATTR_EXCEPTION_DATES_I].value) {
			if ((stat = _ExceptionDateToList((List_node *)node,
			    entry, &appt->exception)) != CSA_SUCCESS) {
				if (stat == CSA_E_INSUFFICIENT_MEMORY) {
					_PrintInsufficientMemoryMsg();
					return(_B_TRUE);
				} else {
					g_failed++;
					_PrintConversionError(entry->key.id,
						(aptr ? aptr->item.string_value
						: NULL), stat);
					return (_B_FALSE);
				}
			}
		}

		/* insert appt */
		appt->appt_id.key = entry->key.id;
		if (stat = _DtCmsInsertAppt(g_newcal, appt)) {
			if (stat == CSA_E_INSUFFICIENT_MEMORY) {
				_PrintInsufficientMemoryMsg();
				return(_B_TRUE);
			} else {
				_DtCm_free_appt4(appt);
				aptr = entry->attrs[CSA_ENTRY_ATTR_SUMMARY_I].value;
				g_failed++;
				_PrintConversionError(entry->key.id,
					(aptr ? aptr->item.string_value : NULL),
					stat);
				return (_B_FALSE);
			}
		}

		if (appt->period.period == single_4)
			g_converted_onetime++;
		else
			g_converted_repeating++;
		return (_B_FALSE);
	}
}

/*
 * if the appointment falls on or after the start date,
 * insert it in the new calendar data structure
 */
static boolean_t
_TrimAppts(caddr_t node, caddr_t a)
{
	CSA_return_code	stat;
	Appt_4		*appt = (Appt_4 *)a;
	boolean_t	wasonetime;

	g_total++;

	if (_IncludeAppt(appt, &wasonetime) == _B_FALSE)
		return (_B_FALSE);

	/* insert appointment in the new tree */
	if (stat = _DtCmsInsertAppt(g_newcal, appt)) {
		g_failed++;
		_PrintConversionError(appt->appt_id.key, appt->what, stat);
		return (_B_TRUE);
	} else {
		if (wasonetime == _B_TRUE)
			g_converted_onetime++;
		else
			g_converted_repeating++;
		return (_B_FALSE);
	}
}

/*
 * if the entry falls on or after the start date,
 * insert it in the new calendar data structure
 */
static boolean_t
_TrimEntries(caddr_t node, caddr_t e)
{
	CSA_return_code	stat;
	cms_entry	*entry = (cms_entry *)e;
	boolean_t	wasonetime;
	cms_attribute_value *aptr;

	g_total++;

	if (_IncludeEntry(node, entry, &wasonetime) == _B_FALSE)
		return (_B_FALSE);

	/* insert appointment in the new tree */
	if (stat = _DtCmsInsertEntry(g_newcal, entry)) {
		g_failed++;
		aptr = entry->attrs[CSA_ENTRY_ATTR_SUMMARY_I].value;
		_PrintConversionError(entry->key.id,
			(aptr ? aptr->item.string_value : NULL), stat);
		return (_B_TRUE);
	} else {
		if (wasonetime == _B_TRUE)
			g_converted_onetime++;
		else
			g_converted_repeating++;
		return (_B_FALSE);
	}
}

static boolean_t
_CountAppts(caddr_t node, caddr_t a)
{
	Appt_4		*appt = (Appt_4 *)a;

	g_total++;

	if (appt->period.period == single_4)
		g_onetime++;
	else
		g_repeating++;

	return (_B_FALSE);
}

static boolean_t
_CountEntries(caddr_t node, caddr_t e)
{
	cms_entry	*entry = (cms_entry *)e;

	g_total++;

	if (entry->attrs[CSA_ENTRY_ATTR_RECURRENCE_RULE_I].value == NULL)
		g_onetime++;
	else
		g_repeating++;

	return (_B_FALSE);
}

/*
 * When converting v3 to v4 data format, initialize the
 * calendar will the corresponding calendar attribute values
 */
static CSA_return_code
_SetCalendarAttrs(_DtCmsCalendar *cal, Access_Entry_4 *oldlist)
{
	cms_attribute_value	attrval;
	cms_access_entry	*alist;
	char			datestr[30];

	/* fill in the current time for date of creation */
	_csa_tick_to_iso8601(time(0), datestr);
	if ((_DtCm_set_string_attrval(datestr,
	    &cal->attrs[CSA_CAL_ATTR_DATE_CREATED_I].value,
	    CSA_VALUE_DATE_TIME)) != CSA_SUCCESS) {
		_PrintInsufficientMemoryMsg();
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	/* convert access list */
	if (oldlist && (alist = _DtCmsConvertV4AccessList(oldlist, g_owner))
	    == NULL) {
		_PrintInsufficientMemoryMsg();
		return (CSA_E_INSUFFICIENT_MEMORY);
	} else {
		if (oldlist == NULL) alist = NULL;

		attrval.type = CSA_VALUE_ACCESS_LIST;
		attrval.item.access_list_value = alist;

		if ((_DtCmUpdateAccessListAttrVal(&attrval,
		    &cal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value))
		    != CSA_SUCCESS) {
			_PrintInsufficientMemoryMsg();
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
	}

	/* set product identifier */
	if ((_DtCm_set_string_attrval(_DtCM_PRODUCT_IDENTIFIER,
	    &cal->attrs[CSA_CAL_ATTR_PRODUCT_IDENTIFIER_I].value,
	    CSA_VALUE_STRING)) != CSA_SUCCESS) {
		_PrintInsufficientMemoryMsg();
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	/* set version of CSA spec supported */
	if ((_DtCm_set_string_attrval(_DtCM_SPEC_VERSION_SUPPORTED,
	    &cal->attrs[CSA_CAL_ATTR_VERSION_I].value, CSA_VALUE_STRING))
	    != CSA_SUCCESS) {
		_PrintInsufficientMemoryMsg();
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	/* set character set */
	if (*g_charsetargv != NULL && (_DtCm_set_string_attrval(g_charsetargv,
	    &cal->attrs[CSA_CAL_ATTR_CHARACTER_SET_I].value, CSA_VALUE_STRING))
	    != CSA_SUCCESS) {
		_PrintInsufficientMemoryMsg();
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	return (CSA_SUCCESS);
}

static void
_SendSigHup()
{
	FILE	*fp = NULL;
	int     fd[2];
	char	buf[BUFSIZ];
	char    cmd[BUFSIZ];
	int	pid;
	int	c_pid;
	int	status;
        int     done=0;
        int     tries=0;

	char *env[] = {
                "PATH=/usr/bin:/usr/sbin:/usr/dt/bin:/usr/lib",
                "IFS=' '",
                "LD_LIBRARY_PATH=/usr/lib",
                NULL
        };

	sprintf(cmd, "ps -e|grep rpc.cmsd|grep -v grep");
	
        done = 0;
        tries = 0;

        while ( (!done) && (tries <= 5) ) {

	  pipe(fd);

	  c_pid = fork();
	  if (c_pid == 0) { /* The child. */
		  close(fd[0]);
		  dup2(fd[1], STDOUT_FILENO);
		  close(fd[1]);
		  execle("/usr/bin/sh", "sh", "-c", cmd, (char *)0, env);
		  _exit(1);
	  }
	  else if (c_pid > 0) { /* The parent. */
		  close(fd[1]);
		  fp = fdopen(fd[0], "r");
		  if (fgets(buf, sizeof(buf), fp) != NULL) {
		    pid = atoi(buf);
		    kill(pid, SIGHUP);
		    sleep(5);
                    tries++;
		  }
		  else
		    done = 1;
		  fclose(fp);
		  close(fd[0]);
		  wait(&status);
	  }
       }
}

/*
 * lock the /var/spool/calendar/.lock.host file so that
 * other program will not try to read the calendar files
 */
static int
_DoLock()
{
	char	buff[MAXPATHLEN], errmsg[BUFSIZ];
	int	error;
	int	fd;
	struct flock locker;

	locker.l_type = F_WRLCK;
	locker.l_whence = 0;
	locker.l_start = 0;
	locker.l_len = 0;

	sprintf(buff, "%s/.lock.", _DtCMS_DEFAULT_DIR);

	/* 
	 * /var/spool might be mounted.  Use .lock.hostname to
	 * prevent more than one cms running in each host.
	 */
	strcat(buff, _DtCmGetLocalHost());

	umask(2);
	fd = open(buff, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	if (fd < 0)
	{
		sprintf(errmsg, catgets(g_catd, 1, 23,
			"Failed to lock lockfile, %s.\n"), buff);
		perror(errmsg);
		exit (1);
	}

	if (fcntl (fd, F_SETLK, &locker) != 0) {
		error = errno;

		close(fd);

		if (error != EWOULDBLOCK && error != EACCES) {

			sprintf(errmsg, catgets(g_catd, 1, 24,
				"Failed to lock lockfile, %s.\n"), buff);
			perror(errmsg);
			exit (1);

		} else
			return (_FAILURE);
	}

	return (_OK);
}


/*
 * Prevent rpc.cmsd from starting during conversion.
 */
static int
lock_it()
{
	char *dir = _DtCMS_DEFAULT_DIR;
	char	buff [MAXPATHLEN];
	int	error;
	int	fd;
#ifdef SVR4
	struct flock locker;
	locker.l_type = F_WRLCK;
	locker.l_whence = 0;
	locker.l_start = 0;
	locker.l_len = 0;
#endif /* SVR4 */

	strcpy (buff, dir);
	strcat (buff, "/.lock.convert.");

	/* 
	 * /var/spool might be mounted.  Use .lock.hostname to
	 * prevent more than one cms running in each host.
	 */
	strcat(buff, _DtCmGetLocalHost());

	fd = open(buff, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	if (fd < 0)
	{
		printf("failed to open lock file %s\n", buff);
		exit (-1);
	}

	/*
	 * Note, we have to use flock() instead of lockf() because cms process
	 * is run in each host.
	 */
#ifdef SVR4
	if (fcntl (fd, F_SETLK, &locker) != 0)
#else
	if (flock (fd, LOCK_EX|LOCK_NB) != 0)
#endif /* SVR4 */
	{
		error = errno;

		close(fd);

		if (error != EWOULDBLOCK && error != EACCES) {

			printf("failed to lock lockfile: %s\n",
					strerror(error));
			exit (-1);

		} else {
			if (debug)
				fprintf(stderr, "rpc.cmsd: %s\n",
				    "lock_it failed due to another process");
			
			/* cms has been running.... */
			return(error);
		}
	}

	return (0);
}


static void
_UnLockDirectory()
{
  char    buff[MAXPATHLEN];
  sprintf(buff, "/bin/rm -f %s/.lock.convert.", _DtCMS_DEFAULT_DIR);
  strcat(buff, _DtCmGetLocalHost());
  system(buff);
}


static void
_LockDirectory()
{
	_SWITCH_TO_SUPER_USER;

        if ( lock_it() != 0 )
          exit (1);

	if (_DoLock() == _FAILURE) {
		/* rpc.cmsd is running, send sighup to it */
		_SendSigHup();

		if (_DoLock() == _FAILURE) {
			fprintf(stderr, catgets(g_catd, 1, 25, "Failed to kill rpc.cmsd, please kill it before\nrunning this program.\n"));
			exit (1);
		}
	}

	_SWITCH_TO_REAL_USER;
}

static void
_GetDaemonIds()
{
	struct	passwd *pw;
	struct	group *gr;

	pw = (struct passwd *)getpwnam("daemon");
	gr = (struct group *)getgrnam("daemon");
	if (pw != NULL) 
		daemon_uid = pw->pw_uid;
	else
		daemon_uid = 1;
	if (gr != NULL)
		daemon_gid = gr->gr_gid;
	else 
		daemon_gid = 1;
}

/*
 * This routine make sure that the calling user is either the
 * the super-user or the owner of the calendar.
 * The user is the owner
 * - if cal name is a user name then it must be the calling user's, or
 * - if the cal name is not a user name, the uid of the calendar file
 *   must be the same as the calling user's
 * This assumes that the ownership of the file matches that of the
 * the calendar.
 */
static int
_CheckUser()
{
	struct	passwd *pw;

	/* check wither the calendar name is a valid user name */
	if (pw = getpwnam(g_calname))
		g_useruid = pw->pw_uid;

	/* must be super-user or the owner of the calendars */
	if (g_uid != 0 &&
	    ((pw && pw->pw_uid != g_uid) || (g_owneruid != g_uid)))
		return (_FAILURE);
	else
		return (_OK);
}

static int
_CopyFile(char *from, char *to)
{
	int	ffd, tfd, nr, nw;
	char	buf[BUFSIZ];
	struct	stat	finfo;

	if ((ffd = open(from, O_RDONLY)) < 0) {
		fprintf(stderr, catgets(g_catd, 1, 26, "Failed to open %s\n"),
			from);
		return (_FAILURE);
	}

	/* if calorig.name is a sym link, remove it first */
	if ((lstat(to, &finfo) == 0) && (finfo.st_mode & S_IFLNK) == S_IFLNK) {
		unlink(to);
	}

	if ((tfd = open(to, O_WRONLY|O_CREAT|O_TRUNC|O_SYNC,
	    _DtCMS_DEFAULT_MODE)) < 0) {
		/* unlink it and try again */
		unlink(to);

		if ((tfd = open(to, O_WRONLY|O_CREAT|O_TRUNC|O_SYNC,
		    _DtCMS_DEFAULT_MODE)) < 0) {
			fprintf(stderr, catgets(g_catd, 1, 27,
				"Failed to open %s\n"), to);
			return (_FAILURE);
		}
	}

	while ((nr = read(ffd, buf, BUFSIZ)) > 0) {
		nw = write(tfd, buf, nr);
		if (nw != nr) {
			close(ffd);
			close(tfd);
			fprintf(stderr, catgets(g_catd, 1, 29,
				"Failed to backup calendar file to %s\n"),
				to);
			return (_FAILURE);
		}
	}

	close(ffd);

	_SWITCH_TO_SUPER_USER;

	if (_DtCmsSetFileMode(to, tfd, g_owneruid, daemon_gid,
	    _DtCMS_DEFAULT_MODE, _B_FALSE, _B_FALSE) < 0) {
		fprintf(stderr, catgets(g_catd, 1, 28,
			"Failed to set %s to the correct mode and/or ownership.\n"),
			to);
		return (_FAILURE);
	}

	_SWITCH_TO_REAL_USER;

	close(tfd);

	return (_OK);
}

static int
_CheckDiskSpace(char *dir, int size)
{
	FILE	*fp = NULL;
	int	fd[2];
	char	buf[BUFSIZ];
	char    cmd[BUFSIZ];
	int	c_pid;
	int	count = 0;
	unsigned long num = 0;
	int	status;

	char *env[] = {
                "PATH=/usr/bin:/usr/sbin:/usr/dt/bin:/usr/lib",
                "IFS=' '",
                "LD_LIBRARY_PATH=/usr/lib",
                 NULL
        };

	sprintf(cmd, "df -k %s", dir);

	pipe(fd);

	c_pid = fork();
        if (c_pid == 0) { /* The child. */
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                execle("/usr/bin/sh", "sh", "-c", cmd, (char *)0, env);
                _exit(1);
        }
        else if (c_pid > 0) { /* The parent. */
                close(fd[1]);
                fp = fdopen(fd[0], "r");
		while (fgets(buf, sizeof(buf), fp) != NULL) {
			count++;
		}
		fclose(fp);
		close(fd[0]);
		wait(&status);
	}

	if (count != 2)
		return (errno);

	sscanf(buf, "%*s %*d %*d %ul", &num);

	if ((num * 1000 - size) < 0) {
		/*
		 * NL_COMMENT
		 * Attention Translator:
		 * messages number (30, 31 and 32) or number (30, 31 and 34) or
		 * number (30, 31 and 35) are to be printed out together as a
		 * group depending on the error situation
		 */
		fprintf(stderr, catgets(g_catd, 1, 30,
			"There is not enough space on %s\n"), dir);
		fprintf(stderr, catgets(g_catd, 1, 31,
			"Please free up some space before running this program.\n"));
		return (_NO_SPACE);
	} else
		return (_OK);
}

/*
 * if -d is not specified, check that default directory has enough
 * space for newsize
 * otherwise, check that
 * 1. root can write to the backup directory
 * 2. backup directory has enough room for size
 * 3. default directory has enough room for newsize-size 
 *
 * return value:
 * _OK  = everything ok
 * _CANT_ACCESS = root cannot write backup directory
 * _NO_SPACE = not enough disk space
 */
static int
_CheckDirectory(int size, int newsize)
{
	int	fd, res;
	char	buf[BUFSIZ];

	if (*g_bdirargv == NULL) {
		strcpy(g_bdirargv, _DtCMS_DEFAULT_DIR);
		if ((res = _CheckDiskSpace(g_bdirargv, newsize)) == _NO_SPACE) {
			fprintf(stderr, catgets(g_catd, 1, 32,
				"The new calendar file may need as much as %d bytes of disk space.\n"),
				newsize);
		}
		return (res);
	} else {
		g_do_copy = _B_TRUE;

		/* check whether we can write the backup directory */
		sprintf(buf, "%s/.sdtcm_convertXXXXXX", g_bdirargv);
		mktemp(buf);
		if ((fd = open(buf, O_WRONLY|O_CREAT|O_TRUNC|O_SYNC,
		    _DtCMS_DEFAULT_MODE)) < 0) {
			if (errno != EPERM && errno != EACCES) {
				fprintf(stderr, catgets(g_catd, 1, 33,
					"Problem accessing %s: "), g_bdirargv);
				return (errno);
			} else
				return (_CANT_ACCESS);
		} else if (_DtCmsSetFileMode(buf, fd, g_owneruid, daemon_gid,
			    _DtCMS_DEFAULT_MODE, _B_FALSE, _B_FALSE) < 0) {
				close(fd);
				unlink(buf);
				return (_CANT_CHANGE_MODE);
		} else {
			close(fd);
			unlink(buf);
		}

		if ((res = _CheckDiskSpace(g_bdirargv, size)) == _NO_SPACE) {
			fprintf(stderr, catgets(g_catd, 1, 34,
				"The calendar file needs %d bytes of disk space to backup.\n"),
				size);
			return (res);
		} else if (res != _OK)
			return (res);

		if ((res = _CheckDiskSpace(_DtCMS_DEFAULT_DIR, newsize - size))
		    == _NO_SPACE) {
			fprintf(stderr, catgets(g_catd, 1, 35,
				"The new calendar file may need as much as %d bytes of disk space.\n"),
				newsize);
		}
		return (res);
	}
}

static boolean_t
_SameVersion(int fversion, int oversion)
{
	int	ver;

	ver = fversion == 1 ? 3 : fversion;

	if (ver == oversion)
		return (_B_TRUE);
	else
		return (_B_FALSE);
}

static void
_GetRpccmsdVersion()
{
	CLIENT *cl;

	if (cl = clnt_create_vers(_DtCmGetLocalHost(), TABLEPROG,
	    &g_svr_version, 2, TABLEVERS, "udp")) {
		clnt_destroy(cl);
	}
}

static boolean_t
_GetConfirmation(char *msg)
{
	char	res[10];

	fprintf(stdout, msg);
	fgets(res, 10, stdin);
	if (!(*res == '\n' || *res == 'Y' || *res == 'y'))
		return (_B_FALSE);
	else
		return (_B_TRUE);
}

static int
_FixCalendarName(_DtCmsCalendar *cal, char *nameattr)
{
	char		res[10];
	char		*ptr, *log;
	struct stat	info;
	cms_attribute_value *aptr;
	int		statres = -1;

	/*
	 * NL_COMMENT
	 * Attention Translator:
	 * messages number (36, 37 and 38) or number (36 and 39) are
	 * to be printed out together as a group depending on the situation
	 */
	fprintf(stdout, catgets(g_catd, 1, 36, 
		"The calendar name (%s) is inconsistent with that in the calendar name\nattribute (%s)."),
		g_calname, nameattr);

	/* correct the attribute value if the calendar name is a user name
	 * or a calendar with the name in the attribute exists already
	 */
	log = _DtCmsGetLogFN(nameattr);
	if (g_useruid != -1 || (statres = stat(log, &info)) == 0) {

		if (g_useruid == -1 && statres == 0) {
			fprintf(stdout, catgets(g_catd, 1, 37,
				"  There is already a calendar with the name %s"),
				nameattr);
		}

		if (_GetConfirmation(catgets(g_catd, 1, 38,
		    "\nDo you want to correct the value of the calendar name attribute? (Y/N) [Y] "))
		    == _B_FALSE) {
			return (_REJECT_CORRECTION);
		} else
			*res = '1'; /* correct the attribute value */
	} else {
		if (_GetConfirmation(catgets(g_catd, 1, 39, "\nDo you want to correct it? (Y/N) [Y] ")) == _B_FALSE) {
			return (_REJECT_CORRECTION);
		}

		/*
		 * NL_COMMENT
		 * Attention Translator:
		 * messages number 40 and 41 are to be printed out together
		 */
		fprintf(stdout, catgets(g_catd, 1, 40,
			"Please select a calendar name, 1) %s or 2) %s\n"),
			g_calname, nameattr);

		*res = NULL;
		while (*res != '1' && *res != '2') {
			fprintf(stdout, catgets(g_catd, 1, 41,
				"Name to use? (1, 2, or q to quit) "));

			fgets(res, 10, stdin);

			if (*res == 'q' || *res == 'Q')
				exit(0);
			else if (*res == '1' || *res == '2')
				break;
		}
	}

	if (*res == '1') {
		/* correct the attribute value */
		aptr = cal->attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].value;

		if (ptr = strchr(aptr->item.string_value, '@'))
			strcat(g_calname, ptr);

		free(aptr->item.string_value); 

		if ((aptr->item.string_value = strdup(g_calname)) == NULL) {
			_PrintInsufficientMemoryMsg();
			return (_FAILURE);
		}

		if (ptr = strchr(g_calname, '@')) *ptr = NULL;
	} else {
		/* update calendar name variable */
		strcpy(g_calname_old, g_calname);
		strcpy(g_calname, nameattr);
	}

	g_fixit = _B_TRUE;
	return (_OK);
}

static int
_FixOwnership(
	_DtCmsCalendar	*cal,
	char		*ownerattr,
	boolean_t	*need_conf)
{
	char		res[10];
	struct passwd	*pw;
	cms_attribute_value *aptr;

	if (g_useruid != -1) {
		/* calendar name is a valid user name */

		fprintf(stdout, catgets(g_catd, 1, 42,
			"The ownership of the calendar file and/or the calendar owner attribute\nis wrong, the calendar should be owned by %s\n"),
			g_calname);

		if (_GetConfirmation(catgets(g_catd, 1, 43,
		    "Do you want to correct it? (Y/N) [Y] ")) == _B_FALSE) {
			return (_REJECT_CORRECTION);
		}

		if (strcmp(g_calname, ownerattr)) {
			/* correct the attribute value */
			aptr = cal->attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].value;
			free(cal->owner);
			free(aptr->item.calendar_user_value); 
			if ((aptr->item.calendar_user_value = strdup(g_owner))
			    == NULL || (cal->owner = strdup(g_owner)) == NULL) {
				_PrintInsufficientMemoryMsg();
				return (_FAILURE);
			}

			g_fixit = _B_TRUE; /* need to backup original file */
		} else
			*need_conf = _B_FALSE;
	} else {
		/* calendar name is not a valid user name */

		/*
		 * NL_COMMENT
		 * Attention Translator:
		 * messages number (44 and 45) or (44 and 46) are to be
		 * printed out together as a group depending on the
		 * situation
		 */
		fprintf(stdout, catgets(g_catd, 1, 44,
			"Owner (%s) of the calendar file is inconsistent with that in the calendar\nowner attribute (%s).\n"),
			g_owner, ownerattr);

		/* correct the attribute value if user running the program
		 * is not the super user or if the name in the attribute is
		 * not a valid user name.
		 */
		if (g_uid != 0 || (pw = getpwnam(ownerattr)) == NULL) {
			if (_GetConfirmation(catgets(g_catd, 1, 45, 
			    "Do you want to correct the value of the calendar owner attribute? (Y/N) [Y] "))
			    == _B_FALSE) {
				return (_REJECT_CORRECTION);
			} else
				*res = '1'; /* correct the attribute value */
		} else {
			/* super user can pick the owner */
			if (_GetConfirmation(catgets(g_catd, 1, 46,
			    "Do you want to correct it? (Y/N) [Y] "))
			    == _B_FALSE) {
				return (_REJECT_CORRECTION);
			}

			/*
			 * NL_COMMENT
			 * Attention Translator:
			 * messages number 47 and 48 are to be printed out
			 * together
			 */
			fprintf(stdout, catgets(g_catd, 1, 47,
				"Please select an owner, 1) %s or 2) %s\n"),
				g_owner, ownerattr);

			*res = NULL;
			while (*res != '1' && *res != '2') {
				fprintf(stdout, catgets(g_catd, 1, 48,
					"Name to use? (1, 2, or q to quit) "));

				fgets(res, 10, stdin);

				if (*res == 'q' || *res == 'Q')
					exit(0);
				else if (*res == '1' || *res == '2')
					break;
			}
		}

		if (*res == '1') {
			/* correct the attribute value */
			aptr = cal->attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].value;
			free(cal->owner);
			free(aptr->item.calendar_user_value); 
			if ((aptr->item.calendar_user_value = strdup(g_owner))
			    == NULL || (cal->owner = strdup(g_owner)) == NULL) {
				_PrintInsufficientMemoryMsg();
				return (_FAILURE);
			}

			g_fixit = _B_TRUE; /* need to backup original file */
		} else {
			/* update global variable */
			if (pw = getpwnam(ownerattr)) {
				g_owneruid_old = g_owneruid;
				g_owneruid = pw->pw_uid;
				strcpy(g_owner, ownerattr);
				*need_conf = _B_FALSE;
			} else {
				fprintf(stderr, catgets(g_catd, 1, 49,
					"Failed to get uid for %s\n"),
					ownerattr);
				return (_FAILURE);
			}
		}
	}

	return (_OK);
}

/*
 * For version 3 data:
 * - check file mode
 * - if the calendar name is a valid user name, check file ownership
 * - otherwise if need to do conversion, get the owner name
 * - if correction is needed and done, the routine will exit.
 *
 * For verson 4 data:
 * - check file mode
 * - check whether the specified calendar name is the same as that in calendar
 *	name attribute
 * - check whether file ownership is consistent with that in the calendar
 *	owner attribute
 * - correction other than file mode needs to backup original file and
 *	will be done by other part of the code.
 *
 * return values:
 * _OK - everything is fine
 * _REJECT_CORRECTION - user refuse to do correction
 * _CORRECTION_DONE - correction done successfully
 */
static int
_SanityCheck(_DtCmsCalendar *cal, char *logname)
{
	struct passwd	*pw;
	boolean_t	wrong_mode = _B_FALSE;
	boolean_t	wrong_ouid = _B_FALSE;
	boolean_t	wrong_guid = _B_FALSE;
	boolean_t	wrong_name = _B_FALSE;
	boolean_t	need_conf = _B_TRUE;
	char		nameattr[BUFSIZ];
	char		ownerattr[BUFSIZ];
	char		*ptr;
	int		res;
	int		fd;

	if ((g_filemode & (S_IRWXO | S_IRWXG | S_IRWXU)) != _DtCMS_DEFAULT_MODE)
		wrong_mode = _B_TRUE;

	if (g_grpuid != daemon_gid)
		wrong_guid = _B_TRUE;

	if (cal->fversion == _DtCMS_VERSION1) {

		if (g_useruid != -1) {
			if (g_useruid != g_owneruid) {
				wrong_ouid = _B_TRUE;
				g_owneruid = g_useruid;
			}
			strcpy(g_owner, g_calname);
		} else if (pw = getpwuid(g_owneruid))
			strcpy(g_owner, pw->pw_name);
		else {
			fprintf(stderr, catgets(g_catd, 1, 50,
				"Problem getting the owner name for %s: \n"),
				logname);
			perror(NULL);
			return (_FAILURE);
		}

	} else {
		/* calendar name */
		strcpy(nameattr, cal->attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].\
			value->item.string_value);
		if (ptr = strchr(nameattr, '@')) *ptr = NULL;
		if (strcmp(nameattr, g_calname)) {
			if ((res = _FixCalendarName(cal, nameattr)) != _OK)
				return (res);
		}

		/* calendar owner */
		strcpy(ownerattr, cal->attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].\
			value->item.calendar_user_value);
		if (ptr = strchr(ownerattr, '@')) *ptr = NULL;

		if (g_useruid != -1) {
			strcpy(g_owner, g_calname);

			/* owner should be the same as calendar name */
			if (strcmp(g_calname, ownerattr) ||
			    g_useruid != g_owneruid) {
				g_owneruid = g_useruid;
				wrong_ouid = _B_TRUE;
			}
		} else if (pw = getpwuid(g_owneruid)) {
			if (strcmp(pw->pw_name, ownerattr))
				wrong_ouid = _B_TRUE;
			strcpy(g_owner, pw->pw_name);
		} else {
			fprintf(stderr, catgets(g_catd, 1, 51,
				"Problem getting the owner name for %s: \n"),
				logname);
			perror(NULL);
			return (_FAILURE);
		}

		if (wrong_ouid &&
		    (res = _FixOwnership(cal, ownerattr, &need_conf)) != _OK)
			return (res);
	}

	if (g_fixit == _B_FALSE && (wrong_ouid || wrong_guid || wrong_mode)) {
		if (need_conf) {
			fd = open(logname, O_RDONLY);
			/*
			 * NL_COMMENT
			 * Attention Translator:
			 * messages number 52 and 53 are to be printed out
			 * together
			 */
			fprintf(stdout, catgets(g_catd, 1, 52,
				"The ownership and/or the permission mode of the calendar file is wrong.\n"));

			if (_GetConfirmation(catgets(g_catd, 1, 53,
			    "Do you want to correct it ? (Y/N) [Y] "))
			    == _B_FALSE) {
				return (_REJECT_CORRECTION);
			}
		}

		if (_DtCmsSetFileMode(logname, fd, g_owneruid, daemon_gid,
		    _DtCMS_DEFAULT_MODE, _B_FALSE, _B_FALSE) < 0) {
			fprintf(stderr, catgets(g_catd, 1, 54,
				"Failed to do the correction.\n"));
			return (_FAILURE);
		} else
			return (_CORRECTION_DONE);
	}

	return (_OK);
}

static void
_PrintStat()
{
	fprintf(stdout, catgets(g_catd, 1, 55,
		"Total number of appointments\t\t= %d\n"), g_total);
	fprintf(stdout, catgets(g_catd, 1, 56,
		"Number of one-time appointments\t\t= %d\n"), g_onetime);
	fprintf(stdout, catgets(g_catd, 1, 57,
		"Number of repeating appointments\t= %d\n"), g_repeating);
}

static void
sigsegv_handler(int dummy)
{
  _UnLockDirectory();
}

void
main(argc, argv)
int argc;
char **argv;
{
	_DtCmsCalendar	*oldcal, *cal;
	char		backupfile[BUFSIZ];
	char		*log, *old_log = NULL, *temp, *ptr;
	struct stat 	info;
	int		c, size, newsize;

        signal(SIGSEGV, sigsegv_handler);

        atexit(_UnLockDirectory);

	/* get real user id */
	g_uid = getuid();
	_SWITCH_TO_REAL_USER;

	if (pgname = strrchr(argv[0], '/'))
		pgname++;
	else
		pgname = argv[0];

	/* open the message catalog for internatiionalizaton */
	setlocale(LC_ALL, "");
	g_catd = catopen(SDTCM_CONVERT_CAT, NL_CAT_LOCALE);

	/* get and check arguments */
	if (get_args(argc, argv) == _FAILURE)
		exit(1);

	if (g_startdate == -1 && g_output_ver == -1)
		g_sanity_check = _B_TRUE;

	/* if host name is specified, it has to be the local host */
	if (ptr = strchr(g_calname, '@')) {
		*ptr = NULL;
		if (strcmp(++ptr, _DtCmGetLocalHost())) {
			/*
			 * NL_COMMENT
			 * Attention Translator:
			 * Messages number 59 and 60 will be printed out
			 * together.  If they are printed, they will be
			 * followed by messages number 1 to 5.
			 */
			fprintf(stderr, catgets(g_catd, 1, 59,
				"You have specified a calendar in a remote host, %s\n"),
				ptr);
			fprintf(stderr, catgets(g_catd, 1, 60,
				"The calendar must be located in the local host, %s\n"),
				_DtCmGetLocalHost());
			_PrintUsage();
			exit(1);
		}
	}

	/* check existence of file in local system */
	log = _DtCmsGetLogFN(g_calname);
	if (stat(log, &info) != 0) {
		fprintf(stderr, catgets(g_catd, 1, 61,
			"Problem accessing calendar file %s:\n"), log);
		perror(NULL);
		exit(1);
	} else {
		g_owneruid = info.st_uid;
		g_grpuid = info.st_gid;
		g_filemode = info.st_mode;
		size = info.st_size;
	}

	/* check to make sure calling user has access to the calendar */
	if (_CheckUser() == _FAILURE) {
		fprintf(stderr, catgets(g_catd, 1, 62,
			"%s: Must be run by super-user or the owner of the calendar.\n"),
			pgname);
		exit(1);
	}

	/* get rpc.cmsd version */
	_GetRpccmsdVersion();

	/* make sure rpc.cmsd is not running */
	_LockDirectory();

	/* do some intiialization */
	g_now = time(0);
	init_time();
	_DtCm_init_hash();
	_GetDaemonIds();

	/* print out progress */
	fprintf(stdout, catgets(g_catd, 1, 63, "Loading the calendar ...\n"));

	_SWITCH_TO_SUPER_USER;

	/* load the specified calendar */
	if (_DtCmsGetCalendarByName(g_calname, &oldcal) != CSA_SUCCESS) {
		fprintf(stderr, catgets(g_catd, 1, 64,
			"Failed to parse the calendar file %s\n"), log);
		exit(1);
	}
	_SWITCH_TO_REAL_USER;

	if (g_output_ver == -1)
		g_output_ver = (oldcal->fversion == 1 ? 3 : 4);
	else if (_SameVersion(oldcal->fversion, g_output_ver) == _B_TRUE &&
	    g_startdate == -1)
		g_sanity_check = _B_TRUE;

	if (g_sanity_check == _B_TRUE) {
		if (oldcal->fversion == 1)
			_DtCmsEnumerateUp(oldcal, _CountAppts);
		else
			_DtCmsEnumerateUp(oldcal, _CountEntries);
	}

	/* do sanity check */
	switch (_SanityCheck(oldcal, log)) {
	case _OK:
		if (g_sanity_check == _B_TRUE && g_fixit == _B_FALSE) {
			fprintf(stdout, catgets(g_catd, 1, 65,
				"The calendar file is ok.\n"));
			_PrintStat();
			exit(0);
		}

		if (*g_calname_old) {
			old_log = log;
			log = _DtCmsGetLogFN(g_calname);
		}

		break;

	case _FAILURE:
		exit(1);

	case _REJECT_CORRECTION:
		exit(0);

	case _CORRECTION_DONE:
		fprintf(stdout, catgets(g_catd, 1, 66, "Correction done.\n"));

		if (g_sanity_check == _B_TRUE) {
			_PrintStat();
			exit(0);
		}
		break;
	}

	/* find out estimate of disk space requirement */
	if ((g_output_ver == 4 && oldcal->fversion == 4) ||
	    (g_output_ver == 3 && oldcal->fversion == 1)) {
		newsize = size;
	} else if (g_output_ver == 4)
		newsize = size * 2 + _V4OVERHEAD;
	else
		newsize = size / 1.5;

	/* check directory and available disk space */
	_SWITCH_TO_SUPER_USER;
	switch (_CheckDirectory(size, newsize)) {
	case _OK:
		break;
	case _CANT_ACCESS:
		fprintf(stderr, catgets(g_catd, 1, 67,
			"Do not have permission to write to %s\n"), 
			g_bdirargv);
		fprintf(stderr, catgets(g_catd, 1, -1,
			"Please choose another backup directory.\n"));
		exit(1);
	case _CANT_CHANGE_MODE:
		fprintf(stderr, catgets(g_catd, 1, 67,
			"Do not have permission to change the ownership and/or the permission of\nfiles in %s\n"), 
			g_bdirargv);
		fprintf(stderr, catgets(g_catd, 1, -1,
			"Please choose another backup directory.\n"));
		exit(1);
	case _NO_SPACE:
		exit(1);
	default: /* errno */
		perror(NULL);
		exit(1);
	}
	_SWITCH_TO_REAL_USER;
	sprintf(backupfile, "%s/%s%s", g_bdirargv, "calorig.",
		(*g_calname_old ? g_calname_old : g_calname));

	/* attribute values */
	if (oldcal->fversion == 1 && g_output_ver == 4) {

		/* print out warning if rpc.cmsd does not support v5 */
		if (g_svr_version > 0 &&
		    g_svr_version < _DtCM_FIRST_EXTENSIBLE_SERVER_VERSION) {
			/*
			 * NL_COMMENT
			 * Attention Translator:
			 * Messages 68 and 69 will be printed out together
			 */
			fprintf(stdout, catgets(g_catd, 1, 68,
				"The calendar server, rpc.cmsd, running on your machine does not support\nthe version 4 data format.  If you convert your calendar to the version 4\ndata format, you have to upgrade your calendar server to one that\nunderstands the version 4 data format.\n\n"));

			if (_GetConfirmation(catgets(g_catd, 1, 69,
			    "Do you want to continue? (Y/N) [Y] ")) == _B_FALSE)
				exit(0);
			fprintf(stdout, "\n");
		}
 
		/* get character set */
		if (*g_charsetargv == NULL) {
			/* get locale of the current environment */
			_DtI18NXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,
				setlocale(LC_CTYPE, NULL), &ptr, NULL, NULL);
			if (ptr) {
				strcpy(g_charsetargv, ptr);
				free(ptr);
			}
		} else {
			/* get the cde platform independent locale */
			_DtI18NXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,
				g_charsetargv, &ptr, NULL, NULL);
			if (ptr == NULL) {
				/*
				 * NL_COMMENT
				 * Attention Translator:
				 * Messages 70 and 71 will be printed out
				 * together
				 */
				fprintf(stderr, catgets(g_catd, 1, 70,
					"Could not get the platform independent locale name for %s\n"),
					g_charsetargv);
				fprintf(stderr, catgets(g_catd, 1, 71,
					"Please specify a valid locale name.\n"));
				exit(1);
			} else {
				strcpy(g_charsetargv, ptr);
				free(ptr);
			}
		}

	} else if (oldcal->fversion == 4 && g_output_ver == 3) {
		/*
		 * NL_COMMENT
		 * Attention Translator:
		 * Messages 72 and 73 will be printed out together
		 */
		fprintf(stdout, catgets(g_catd, 1, 72,
			"\nWARNING!! Data will be lost when converting version 4 data format\nback to version 3 data format.\n\n"));

		if (_GetConfirmation(catgets(g_catd, 1, 73,
		    "Do you want to continue? (Y/N) [Y] ")) == _B_FALSE)
			exit(0);
		fprintf(stdout, "\n");
	}

	/* print out progress */
	if (g_sanity_check == _B_FALSE)
		fprintf(stdout, catgets(g_catd, 1, 74,
			"Doing conversion ...\n"));

	sprintf(g_target, "%s@%s", g_calname, _DtCmGetLocalHost());
	if (g_sanity_check == _B_FALSE && g_output_ver == 4) {
		/* create new calendar; set g_newcal to -1 so a New calendar gets created */
		CSA_return_code stat;
		g_newcal = (_DtCmsCalendar*)-1;
	        if ((stat = _DtCmsAddCalendarToList(g_target, &g_newcal)) != CSA_SUCCESS &&
			stat != CSA_E_CALENDAR_EXISTS) {
	                _PrintInsufficientMemoryMsg();
	                exit(1);
	        }
		if (_DtCmsInitCalendar(g_newcal, (oldcal->fversion == 1 ? g_owner : oldcal->owner)) != CSA_SUCCESS) {
			_PrintInsufficientMemoryMsg();
			exit(1);
		}

		if (oldcal->fversion == 1) {
			/* set calendar attributes */
			if (_SetCalendarAttrs(g_newcal, oldcal->alist))
				exit(1);

			/* convert appointments and insert into new calendar */
			if (_DtCmsEnumerateUp(oldcal, _ConvertApptToEntry))
				exit(1);
		} else {
			/* trim calendar */
			if (_DtCmsEnumerateUp(oldcal, _TrimEntries))
				exit(1);

			/* use value from orginal calendar */
			g_newcal->num_attrs = oldcal->num_attrs;
			g_newcal->attrs = oldcal->attrs;
		}

	} else if (g_sanity_check == _B_FALSE) {
		if ((g_newcal = (_DtCmsCalendar *)calloc(1,
		    sizeof(_DtCmsCalendar))) == NULL) {
			_PrintInsufficientMemoryMsg();
			exit(1);
		}

		g_newcal->fversion = _DtCMS_VERSION1;

		if (!(g_newcal->tree = rb_create(_DtCmsGetApptKey,
		    _DtCmsCompareAppt))) {
			_PrintInsufficientMemoryMsg();
			exit(1);
		}

		if (!(g_newcal->list = hc_create(_DtCmsGetApptKey,
		    _DtCmsCompareRptAppt))) {
			_PrintInsufficientMemoryMsg();
			exit(1);
		}

		g_newcal->cal_tbl = _DtCm_cal_name_tbl;
		g_newcal->entry_tbl = _DtCm_entry_name_tbl;

		/* use value from original calendar */
		g_newcal->calendar = oldcal->calendar;
		g_newcal->owner = oldcal->owner;

		if (oldcal->fversion == _DtCMS_VERSION1) {
			/* use value from original calendar */
			g_newcal->alist = oldcal->alist;

			/* trim calendar */
			if (_DtCmsEnumerateUp(oldcal, _TrimAppts))
				exit(1);
		} else {
			Access_Entry_4 *alist = NULL;

			if (oldcal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value &&
			    oldcal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value->\
			    item.access_list_value) {
				if ((alist = _DtCmsConvertV5AccessList(oldcal->\
				    attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value->\
				    item.access_list_value, _B_FALSE)) == NULL) {
					_PrintInsufficientMemoryMsg();
					exit(1);
				}

				g_newcal->alist = alist;
			}

			/* convert appointments and insert into new calendar */
			if (_DtCmsEnumerateUp(oldcal, _ConvertEntryToAppt))
				exit(1);
		}

	}

	/* save original file */
	if (g_do_copy == _B_TRUE) {

		if (_CopyFile((old_log ? old_log : log), backupfile)) {
			fprintf(stderr, catgets(g_catd, 1, 75,
				"The original calendar file %s is not changed.\n"),
				(old_log ? old_log : log));
			exit(1);
		}
	} else {
		/* just rename the file */
		if (rename((old_log ? old_log : log), backupfile) < 0) {
			fprintf(stderr, catgets(g_catd, 1, 76,
				"Failed to move the calendar file to %s\n"),
				backupfile);
			exit(1);
		}
	}

	/* print out progress */
	if (g_sanity_check == _B_FALSE)
		fprintf(stdout, catgets(g_catd, 1, 77,
			"Writing out new file ...\n"));

	/* write out the new file */
	cal = g_newcal ? g_newcal : oldcal;

	_SWITCH_TO_SUPER_USER;

	if ((cal->fversion == _DtCMS_VERSION1 &&
	    ((g_do_copy == _B_TRUE &&
	    _DtCmsWriteVersionString(log, _DtCMS_VERSION1)) ||
	    (g_do_copy == _B_FALSE && _DtCmsCreateLogV1(cal->owner, log)) ||
	    _DtCmsDumpDataV1(log, cal))) ||
	    (cal->fversion != _DtCMS_VERSION1 &&
	    ((g_do_copy == _B_TRUE &&
	    _DtCmsWriteVersionString(log, _DtCMS_VERSION4)) ||
	    (g_do_copy == _B_FALSE && _DtCmsCreateLogV2(cal->owner, log)) ||
	    _DtCmsDumpDataV2(log, cal)))) {

		/*
		 * NL_COMMENT
		 * Attention Translator:
		 * Messages number (78 and 79) or (78 and 80) or (78 and 81)
		 * or (78 and 82) will be printed out together depending
		 * on the error condition
		 */
		fprintf(stderr, catgets(g_catd, 1, 78,
			"Failed to write the new file.\n"));

		if (g_do_copy == _B_TRUE) {
			if (_CopyFile(backupfile, (old_log ? old_log : log))) {
				fprintf(stderr, catgets(g_catd, 1, 79,
					"You need to recover %s from %s\n"),
					(old_log ? old_log : log), backupfile);
			} else {
				unlink(backupfile);
				fprintf(stderr, catgets(g_catd, 1, 80,
					"The original calendar file %s is not changed.\n"),
					(old_log ? old_log : log));
			}
		} else {
			if (rename(backupfile, (old_log ? old_log : log)) < 0) {
				fprintf(stderr, catgets(g_catd, 1, 81,
					"You need to recover %s from %s\n"),
					(old_log ? old_log : log), backupfile);
			} else {
				fprintf(stderr, catgets(g_catd, 1, 82,
					"The original calendar file %s is not changed.\n"),
					(old_log ? old_log : log));
			}
		}

		exit(1);
	} else if (g_sanity_check == _B_TRUE) {

		/*
		 * NL_COMMENT
		 * Attention Translator:
		 * Messages number 83, may be 84, and 85 will be printed out
		 * together
		 */
		fprintf(stdout, catgets(g_catd, 1, 83, "Correction done.\n"));
		if (old_log)
			fprintf(stdout, catgets(g_catd, 1, 84,
				"The new file is saved in %s\n"), log);
		fprintf(stdout, catgets(g_catd, 1, 85,
			"The original file is saved in %s\n"), backupfile);

		_PrintStat();
	} else {
		char buf[BUFSIZ];

		/* print out statistics */

		/*
		 * NL_COMMENT
		 * Attention Translator:
		 * Either message 86 or 87 will be printed out
		 * together with messages number 88 to 94.
		 */
		if (g_failed)
			strcpy(buf, catgets(g_catd, 1, 86,
				"Conversion done successfully with exceptions noted above.\n"));
		else
			strcpy(buf, catgets(g_catd, 1, 87,
				"Conversion done successfully.\n"));
		fprintf(stdout, "%s", buf); 

		fprintf(stdout, catgets(g_catd, 1, 88,
			"Total number of appointments\t\t\t= %d\n"), g_total);
		fprintf(stdout, catgets(g_catd, 1, 89,
			"Number of one-time appointments converted\t= %d\n"),
			g_converted_onetime);
		fprintf(stdout, catgets(g_catd, 1, 90,
			"Number of repeating appointments converted\t= %d\n"),
			g_converted_repeating);
		fprintf(stdout, catgets(g_catd, 1, 91,
			"Number of one-time appointments pruned\t\t= %d\n"),
			g_onetime - g_converted_onetime);
		fprintf(stdout, catgets(g_catd, 1, 92,
			"Number of repeating appointments pruned\t\t= %d\n"),
			g_repeating - g_converted_repeating);
		if (old_log)
			fprintf(stdout, catgets(g_catd, 1, 93,
				"The new file is saved in %s\n"), log);
		fprintf(stdout, catgets(g_catd, 1, 94,
			"The original file is saved in %s\n"), backupfile);
	}

	exit(0);
}

