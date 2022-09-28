/*******************************************************************************
**
**  cm_insert.c
**
**  $XConsortium: cm_insert.c /main/cde1_maint/5 1995/11/14 16:56:42 drk $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#pragma ident "@(#)cm_insert.c	1.37 97/01/31 SMI"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <rpc/rpc.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <nl_types.h>
#include <locale.h>
#include <LocaleXlate.h>
#include "util.h"
#include "getdate.h"
#include "timeops.h"
#include "props.h"
#include "cm_tty.h"

#ifdef FNS
#include "dtfns.h"
#include "cmfns.h"
#endif
#ifdef AIX_ILS
#include <Dt/Dt.h>
#include <Dt/EnvControl.h>
#endif /* AIX_ILS */

#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE       0
#endif

/* routine to get common locale/charset name */
extern void _DtI18NXlateOpToStdLocale(
     		char       *operation,
     		char       *opLocale,
     		char       **ret_stdLocale,
     		char       **ret_stdLang,
     		char       **ret_stdSet);

int debug = 0;
static nl_catd	DT_catd;
static char cm_target[256] = "";	/* target for table (user@host) */
static char cm_date[256] = "";		/* appointment date */
static char cm_view[16] = "day";	/* view span (day,week,month) */
static char cm_start[16] = "";		/* start time for appointment list*/
static char cm_end[16] = "";		/* end time for appointment list */
static char cm_repeatstr[256] = "One Time";/* repeat period */
static char cm_for[256] = "";		/* number of repeats */
static char cm_what[1024] = "Appointment";
					/* what appointment is about */
static char cm_appt_file[1024] = "";
					/* file to find an appointment template in */
static void cm_args();			/* parse command line */

static char**
grab(char **argv,				/* command line arguments */
    char *buf,				/* buffer for keyed data */
    char stop_key)
{
	if (!argv || !*argv) return(argv);
	cm_strcpy (buf,*argv++);
	while(argv && *argv) {
		if (*(*argv) == stop_key) break;
		cm_strcat(buf," ");
		cm_strcat(buf,*argv++);
	}
	argv--;
	return(argv);
}

static void
cm_args(int argc, char **argv)
{

	while (++argv && *argv) {
		switch(*(*argv+1)) {
		case 't':
		case 'c':
			argv = grab(++argv,cm_target,'-');
			break;
		case 'd':
			argv = grab(++argv,cm_date,'-');
			break;
		case 'v':
			argv = grab(++argv,cm_view,'-');
			break;
		case 's':
			argv = grab(++argv,cm_start,'-');
			break;
		case 'e':
			argv = grab(++argv,cm_end,'-');
			break;
		case 'w':
			argv = grab(++argv,cm_what,'-');
			break;
		case 'a':
			argv = grab(++argv,cm_appt_file,'-');
			break;
		default:
			fprintf(stderr, catgets(DT_catd, 1, 191, "Usage:\n\tdtcm_insert [ -c calendar ] [-d <mm/dd/yy>] [ -v view ]\n"));
			fprintf(stderr, catgets(DT_catd, 1, 192, "                   [-w what string] [-s <HH:MMam/pm>] [-e <HH:MMam/pm>]\n"));
			exit(1);
		}
	}
}

static void
prompt_for_line(const char * prompt, const char * defval, char * buffer)
{
  char input_buf[1024];

  printf(prompt, defval);
  gets(input_buf);
  if (input_buf[0] != '\0' && (input_buf[0] != '\n')) {
    cm_strcpy(buffer, input_buf);
  } else {
    cm_strcpy(buffer, (char*)defval);
  }
  return;
}

static void
prompt_for_insert(Props *prop) {
	char			date_str[BUFSIZ];
	char			what_buffer[BUFSIZ];
	char			buf[BUFSIZ];
	char	*		timecopy;

	int			index;
	int			next;

	boolean_t		valid = _B_FALSE;

	const char	*	NoneStr = catgets(DT_catd, 1, 1106,
						  "None");
	const char	*	endMsg = NoneStr;

	const char     *invalidTimeEntered = catgets(DT_catd, 1, 197,
	     "You have entered an invalid time.  Please try again:\n");

	DisplayType	dt = get_int_prop(prop, CP_DEFAULTDISP);

        format_tick(now(), get_int_prop(prop, CP_DATEORDERING),
		    get_int_prop(prop, CP_DATESEPARATOR), date_str);

	printf(catgets(DT_catd, 1, 193,
		       "Please enter the information for the appointment you wish to add.\nDefaults will be shown in parentheses.\n"));

	prompt_for_line(catgets(DT_catd, 1, 194, "Calendar (%s): "),
			cm_get_credentials(), cm_target);

	prompt_for_line(catgets(DT_catd, 1, 195, "Date (%s): "),
			date_str, cm_date);

	while (valid != _B_TRUE){
	  format_time(now(), dt, cm_start);
	  prompt_for_line(catgets(DT_catd, 1, 196, "Start (%s): "),
			  cm_start, cm_start);
	  if (cm_start[0] != '\0') {
	    timecopy = (char *)cm_strdup(cm_start);
	    if (valid_time(prop, timecopy)) {
	      valid = _B_TRUE;
	    } else {
	      printf("%s", invalidTimeEntered);
	    }
	    free(timecopy);
	  }
	}

	sprintf(buf, "%s %s", date_str, cm_start);
	next = (int) cm_getdate(buf, NULL);
        next = next + hrsec;
	format_time(next, dt, cm_end);

	valid = _B_FALSE;
	if (cm_start[0] != '\0') {
	  endMsg = cm_end;
	}

	while (valid != _B_TRUE){
	  prompt_for_line(catgets(DT_catd, 1, 199, "End (%s): "),
			  endMsg, cm_end);
	  if (cm_end[0] != '\0') {
	    timecopy = (char *)cm_strdup(cm_end);
	    if (valid_time(prop, timecopy)) {
	      valid = _B_TRUE;
	    } else {
	      printf("%s", invalidTimeEntered);
	    }
	    free(timecopy);
	  }
	}

	strcpy(cm_repeatstr, catgets(DT_catd, 1, 200, "One Time"));

	prompt_for_line(catgets(DT_catd, 1, 201, 
			"Repeat (%s): "), cm_repeatstr, cm_repeatstr);

	if (strcmp(cm_repeatstr, catgets(DT_catd, 1, 200, "One Time"))) {
		sprintf(buf, catgets(DT_catd, 1, 203, "no default"));
		prompt_for_line(
			catgets(DT_catd, 1, 204, "For (%s): "), buf, cm_for);
	}

	printf(catgets(DT_catd, 1, 205, 
		"What (you may enter up to 5 lines, use ^D to finish):\n"));
	cm_what[0] = NULL;
	for (index = 0; index < 5; index++)
	{
		gets(what_buffer);
		if (what_buffer[0] == '\000')
			break;
		else
		{
			strcat(cm_what, what_buffer);
			strcat(cm_what, "\\n");
		}
		memset(what_buffer, '\000', 256);
	}
	
}

void
main(int argc, char **argv)
{
	int		cnt, status = 0;
	char		*date = NULL, *view = NULL,
			*target_calname = NULL, *target_caladdr = NULL,
			*start = NULL, *end = NULL, *repeat = NULL,
			*numrepeat = NULL, *what = NULL, *uname, *loc;
	Props		*p = NULL;
	CSA_entry_handle	*list;
	CSA_session_handle	c_handle;
	CSA_return_code		stat;
	CSA_calendar_user	csa_user;
	DisplayType		dt;
	CSA_extension		logon_ext[3];
	char			*app_locale = NULL;
	int			version;
	char			*cal_cs;
	CSA_flags		access;
#ifdef FNS
	char		buf[MAXNAMELEN];
#endif

#ifdef __osf__
        svc_init();
#endif
 
	init_time();
#ifdef  AIX_ILS
        _DtEnvControl(DT_ENV_SET); /* set up environment variables */
#endif  /* AIX_ILS */
	setlocale(LC_ALL, "");
	DT_catd = catopen(DTCM_CAT, NL_CAT_LOCALE);
	cm_tty_load_props(&p);
	dt = get_int_prop(p, CP_DEFAULTDISP);
#ifdef FNS
	dtfns_init();
#endif

	if (argc > 1)
	{
		cm_args(argc,argv);		/* parse command line */
		if (cm_strlen(cm_target)) 
			target_calname = cm_target;
		else
			target_calname = cm_get_credentials();
#ifdef FNS
		if (cmfns_use_fns(p) &&
		    cmfns_lookup_calendar(target_calname, buf, sizeof(buf)) > 0) {
			target_caladdr = buf;
		} else
#endif /* FNS */
			target_caladdr = target_calname;

		if ((uname = cm_target2name(target_caladdr)) == NULL) {
			if (target_caladdr)
				fprintf(stderr, catgets(DT_catd, 1, 620,
					"Unknown calendar. Calendar name needed: <name>%s"),
					target_caladdr);
			else
				fprintf(stderr, catgets(DT_catd, 1, 619,
					"Please enter a calendar name in the format: <user>@<hostname>"));
#ifdef FNS
			if (cmfns_use_fns(p))
				fprintf(stderr, catgets(DT_catd, 1, 618,
					" or supply an FNS name"));
#endif /* FNS */
			fprintf(stderr, "\n");
			exit(1);
		}

		if ((loc = cm_target2location(target_caladdr)) == NULL) {
			fprintf(stderr, catgets(DT_catd, 1, 622,
				"Unknown calendar. Hostname needed: %s@<hostname>"),
				uname);
#ifdef FNS
			if (cmfns_use_fns(p))
				fprintf(stderr, catgets(DT_catd, 1, 618,
					" or supply an FNS name"));
#endif /* FNS */
			fprintf(stderr, "\n");
			free(uname);
			exit(1);
		}

		free(uname);
		free(loc);

		csa_user.user_name = target_caladdr;
		csa_user.user_type = 0;
		csa_user.calendar_user_extensions = NULL;
		csa_user.calendar_address = target_caladdr;

		memset((void *)logon_ext, NULL, sizeof(logon_ext));

		/* get user access */
		logon_ext[0].item_code = CSA_X_DT_GET_USER_ACCESS_EXT;

		/* get calendar's data version */
		logon_ext[1].item_code = CSA_X_DT_GET_DATA_VERSION_EXT;

		/* get calendar's character set attribute */
		logon_ext[2].item_code = CSA_X_DT_GET_CAL_CHARSET_EXT;
		logon_ext[2].extension_flags = CSA_EXT_LAST_ELEMENT;

		stat = csa_logon(NULL, &csa_user, NULL, NULL, NULL, &c_handle,
				 logon_ext);

		if (stat != CSA_SUCCESS) {
		  	char *format = cm_strdup(catgets(DT_catd, 1, 206, 
					   "\nCould not open calendar %s\n"));
			fprintf(stderr, format,
				target_calname ? target_calname : 
				catgets(DT_catd, 1, 209, "UNKNOWN"));
			free(format);
			exit(1);
		}

		access = logon_ext[0].item_data;
		version = logon_ext[1].item_data;
		cal_cs = logon_ext[2].item_reference;

		if ((version < DATAVER4 &&
		    (access & (CSA_X_DT_INSERT_ACCESS|CSA_OWNER_RIGHTS))==0) ||
		    (version >= DATAVER4 &&
		    (access & (CSA_INSERT_PUBLIC_ENTRIES |
			       CSA_INSERT_CONFIDENTIAL_ENTRIES |
			       CSA_INSERT_PRIVATE_ENTRIES |
			       CSA_OWNER_RIGHTS)) == 0)) {

			/* user does not have insert access */
			fprintf(stderr,
				catgets(DT_catd, 0, -1,
				"\nYou do not have insert access for %s\n"),
				target_calname);

			exit(1);
		}

		/* check if appointment needs to be tag with character set */
		if (!cm_appt_file[0] && version >= DATAVER4) {
			/* get locale/charset name of the running process */
			_DtI18NXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,
				setlocale(LC_CTYPE, NULL), &app_locale, NULL,
				NULL);

			if (app_locale &&
			    (cal_cs != NULL && !strcmp(cal_cs, app_locale))) {
				/* don't tag appointment if we have the
				 * same locale/charset as that of the calendar
				 */
				free(app_locale);
				app_locale = NULL;
			}

			if (cal_cs) csa_free(cal_cs);
		}
	
		if (!cm_date[0])
        		format_tick(now(), get_int_prop(p, CP_DATEORDERING),
		    		    get_int_prop(p, CP_DATESEPARATOR), cm_date);
		if (cm_strlen(cm_date)) date = cm_date;
		if (cm_strlen(cm_view)) view = cm_view;
		if (cm_strlen(cm_start)) start = cm_start;

		if (cm_end[0] == '\0' && cm_start[0] != '\0') {
			format_time((int)cm_getdate(cm_start, NULL) + hrsec,
				    dt, cm_end);
		}

		if (cm_strlen(cm_end)) end = cm_end;
		if (cm_strlen(cm_repeatstr)) repeat = cm_repeatstr;
		if (cm_strlen(cm_for)) numrepeat = cm_for;
		if (cm_strlen(cm_what)) what = cm_what;
		if (!cm_appt_file[0])
			status = cm_tty_insert(DT_catd, c_handle, version, 
				      date, start, end, repeat, numrepeat,
				      what, NULL, p, app_locale);
		else
			status = cm_tty_insert(DT_catd, c_handle, version, date,
				      start, end, repeat, numrepeat,
				      what, cm_appt_file, p, NULL);
	} else {
		prompt_for_insert(p);
		if (cm_strlen(cm_target)) target_calname = cm_target;

#ifdef FNS
		if (cmfns_use_fns(p) &&
		    cmfns_lookup_calendar(target_calname, buf, sizeof(buf)) > 0) {
			target_caladdr = buf;
		} else
#endif /* FNS */
			target_caladdr = target_calname;

		if ((uname = cm_target2name(target_caladdr)) == NULL) {
			if (target_caladdr)
				fprintf(stderr, catgets(DT_catd, 1, 620,
					"Unknown calendar. Calendar name needed: <name>%s"),
					target_caladdr);
			else
				fprintf(stderr, catgets(DT_catd, 1, 619,
					"Please enter a calendar name in the format: <user>@<hostname>"));
#ifdef FNS
			if (cmfns_use_fns(p))
				fprintf(stderr, catgets(DT_catd, 1, 618,
					" or supply an FNS name"));
#endif /* FNS */
			fprintf(stderr, "\n");
			exit(1);
		}

		if ((loc = cm_target2location(target_caladdr)) == NULL) {
			fprintf(stderr, catgets(DT_catd, 1, 622,
				"Unknown calendar. Hostname needed: %s@<hostname>"),
				uname);
			free(uname);
#ifdef FNS
			if (cmfns_use_fns(p))
				fprintf(stderr, catgets(DT_catd, 1, 618,
					" or supply an FNS name"));
#endif /* FNS */
			fprintf(stderr, "\n");
			exit(1);
		}

		free(uname);
		free(loc);

		csa_user.user_name = target_caladdr;
		csa_user.user_type = 0;
		csa_user.calendar_user_extensions = NULL;
		csa_user.calendar_address = target_caladdr;

		memset((void *)logon_ext, NULL, sizeof(logon_ext));

		/* get user access */
		logon_ext[0].item_code = CSA_X_DT_GET_USER_ACCESS_EXT;

		/* get calendar's data version */
		logon_ext[1].item_code = CSA_X_DT_GET_DATA_VERSION_EXT;

		/* get calendar's character set attribute */
		logon_ext[2].item_code = CSA_X_DT_GET_CAL_CHARSET_EXT;
		logon_ext[2].extension_flags = CSA_EXT_LAST_ELEMENT;

		stat = csa_logon(NULL, &csa_user, NULL, NULL, NULL, &c_handle,
				 logon_ext);

		if (stat !=CSA_SUCCESS) {
		  	char *format = cm_strdup(catgets(DT_catd, 1, 206, 
					   "\nCould not open calendar %s\n"));
			fprintf(stderr, format, 
				target_calname ? target_calname : 
				catgets(DT_catd, 1, 209, "UNKNOWN"));
			free(format);
			exit(1);
		}

		access = logon_ext[0].item_data;
		version = logon_ext[1].item_data;
		cal_cs = logon_ext[2].item_reference;

		if ((version < DATAVER4 &&
		    (access & (CSA_X_DT_INSERT_ACCESS|CSA_OWNER_RIGHTS))==0) ||
		    (version >= DATAVER4 &&
		    (access & (CSA_INSERT_PUBLIC_ENTRIES |
			       CSA_INSERT_CONFIDENTIAL_ENTRIES |
			       CSA_INSERT_PRIVATE_ENTRIES |
			       CSA_OWNER_RIGHTS)) == 0)) {

			/* user does not have insert access */
			fprintf(stderr,
				catgets(DT_catd, 0, -1,
				"\nYou do not have insert access for %s\n"),
				target_calname);

			exit(1);
		}

		/* check if appointment needs to be tag with character set */
		if (version >= DATAVER4) {
			/* get locale/charset name of the running process */
			_DtI18NXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,
				setlocale(LC_CTYPE, NULL), &app_locale, NULL,
				NULL);

			if (app_locale &&
			    (cal_cs != NULL && !strcmp(cal_cs, app_locale))) {
				/* don't tag appointment if we have the
				 * same locale/charset as that of the calendar
				 */
				free(app_locale);
				app_locale = NULL;
			}

			if (cal_cs) csa_free(cal_cs);
		}
	
		if (cm_strlen(cm_date)) date = cm_date;
		if (cm_strlen(cm_view)) view = cm_view;
		if (cm_strlen(cm_start)) start = cm_start;
		if (cm_strlen(cm_end)) end = cm_end;
		if (cm_strlen(cm_repeatstr)) repeat = cm_repeatstr;
		if (cm_strlen(cm_for)) numrepeat = cm_for;
		if (cm_strlen(cm_what)) what = cm_what;
		status = cm_tty_insert(DT_catd, c_handle, version, date, 
			      start, end, repeat, numrepeat, what, NULL, p,
			      app_locale);
	}

	if (status == CSA_SUCCESS) {
		if ((cnt = cm_tty_lookup(DT_catd, c_handle, version, date,
		    view, &list, p)) > 0)
			csa_free(list);
	}

	csa_logoff(c_handle, NULL);
	props_clean_up(p);
	free(p);
        exit(status);
}
