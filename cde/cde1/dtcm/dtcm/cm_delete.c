/*******************************************************************************
**
**  cm_delete.c
**
**  $XConsortium: cm_delete.c /main/cde1_maint/5 1995/11/14 16:56:25 drk $
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

#pragma ident "@(#)cm_delete.c	1.33 96/11/12 SMI"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <rpc/rpc.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <csa.h>
#include <nl_types.h>
#include <locale.h>
#include <Dt/Dt.h>
#include <Dt/EnvControl.h>
#include "util.h"
#include "timeops.h"
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

int debug = 0;
static nl_catd	DT_catd;
static char cm_target[256] = "";	/* target for table (user@host) */
static char cm_date[256] = "";		/* appointment date */
static char cm_view[16] = "day";		/* view span (day,week,month) */
static int cm_index = 0;		/* index to change/delete */

static char**
grab(char**argv,				/* command line arguments */
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
		default:
			fprintf(stderr,catgets(DT_catd, 1, 187, "Usage:\n\tdtcm_delete [-c calendar] [-d <mm/dd/yy>] [-v view]\n"));
			exit(1);
		}
	}
}

int main(int argc, char **argv)
{
	int		cnt;
	char		index[10], *target_calname = NULL, *target_caladdr = NULL,
			*date = NULL, *view = NULL,
			*uname, *loc;
	Props		*p = NULL;
	CSA_entry_handle	*list;
	CSA_session_handle	c_handle;
	CSA_return_code		stat;
	CSA_calendar_user	csa_user;
	int			version;
#ifdef FNS
	char		buf[MAXNAMELEN];
#endif

#ifdef __osf__
        svc_init();
#endif
 
	init_time();
	setlocale(LC_ALL, "");
	_DtEnvControl(DT_ENV_SET); /* set up environment variables */
	DT_catd = catopen(DTCM_CAT, NL_CAT_LOCALE);
	cm_tty_load_props(&p);
	cm_args(argc,argv);		/* parse command line */
	target_calname = (cm_strlen(cm_target)) ? cm_target : cm_get_credentials();

#ifdef FNS
	dtfns_init();
	if (cmfns_use_fns(p) &&
	    cmfns_lookup_calendar(target_calname, buf, sizeof(buf)) > 0) {
		target_caladdr = buf;
	} else
#endif /* FNS */
		target_caladdr = target_calname;

	if (cm_strlen(cm_date)) date = cm_date;
	if (cm_strlen(cm_view)) view = cm_view;

	uname = cm_target2name(target_caladdr);
	loc = cm_target2location(target_caladdr);

	csa_user.user_name = target_caladdr;
	csa_user.user_type = 0;
	csa_user.calendar_user_extensions = NULL;
	csa_user.calendar_address = target_caladdr;
	stat = csa_logon(NULL, &csa_user, NULL, NULL, NULL, &c_handle, NULL);

	if (stat != CSA_SUCCESS) {
	  	char *format = strdup(catgets(DT_catd, 1, 188,
					   "\nCould not open calendar %s\n"));
		fprintf(stderr, format,
			target_calname ? target_calname : catgets(DT_catd, 1, 189, "UNKNOWN"));
		free(format);
		free(uname);
		free(loc);
		exit(1);
	}
	version = get_data_version(c_handle);

	while (!cm_index) {
		if ((cnt = cm_tty_lookup(DT_catd, c_handle, version, date, 
					view, &list, p)) <= 0) {
			csa_logoff(c_handle, NULL);
			free(uname);
			free(loc);
			exit(0);
		}
		fprintf(stdout, catgets(DT_catd, 1, 190,
				       "Item to delete (number)? "));
		fgets(index, 9, stdin);
		fprintf(stdout,"\n\n");
		if (*index < '0' || *index > '9') {
			csa_free(list);
			break;
		}
		cm_index = atoi(index);
		if (cnt >= cm_index)
			cm_tty_delete(DT_catd, c_handle, version, 
				      cm_index - 1, list);
		memset(index, '\0', 10);
		csa_free(list);
		cm_index = 0;
	}

	csa_logoff(c_handle, NULL);
	props_clean_up(p);
	free(p);
	free(uname);
	free(loc);
	return 0;
}
