/*******************************************************************************
**
**  cm_lookup.c
**
**  $XConsortium: cm_admin.c /main/cde1_maint/3 1995/11/14 16:56:07 drk $
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

#pragma ident "@(#)cm_admin.c	1.13 96/11/12 Sun Microsystems, Inc."

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
#include "cm_tty.h"
#include "util.h"
#include "timeops.h"
#ifdef FNS
#include "dtfns.h"
#include "cmfns.h"
#endif

#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE       0
#endif

typedef enum {archive, restore} Admin_mode;

static Admin_mode Mode = archive;
static char cm_target[MAXPATHLEN];
static char cm_start[MAXPATHLEN];
static char cm_end[MAXPATHLEN];
static char cm_file[MAXPATHLEN];
static int Delete = _B_FALSE;
static FILE *input_stream = stdin;
static FILE *output_stream = stdout;

int debug = 0;
static nl_catd	DT_catd;

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
usage_msg() {
	fprintf(stderr,catgets(DT_catd, 1, 696, "Usage:\n\tdtcm_admin [ -d ] [ -a action ] [ -c calendar ][ -s <mm/dd/yy> ] [ -e <mm/dd/yy> ] [-f filename]\n"));
}

static void
cm_args(int argc, char **argv)
{

	while (++argv && *argv) {
		switch(*(*argv+1)) {
		case 'a':
			++argv;
			if (strcmp(*argv, "archive") == 0)
				Mode = archive;
			else if (strcmp(*argv, "restore") == 0)
				Mode = restore;
			else {
				fprintf(stderr,catgets(DT_catd, 1, 697, "You have specified an unsupported administration operation \"%s\".\n"), *argv);
				usage_msg();
				exit(1);
			}
			break;
		case 'c':
			argv = grab(++argv,cm_target,'-');
			break;
		case 's':
			argv = grab(++argv,cm_start,'-');
			break;
		case 'e':
			argv = grab(++argv,cm_end,'-');
			break;
		case 'f':
			argv = grab(++argv,cm_file,'-');
			break;
		case 'd':
			Delete = _B_TRUE;
			break;

		case 'h':
		default:
			usage_msg();
			exit(1);
		}
	}
}

int main(int argc, char **argv)
{
	int		cnt;
	char		*target_calname = NULL, *target_caladdr = NULL,
			*date = NULL, *view = NULL,
			*uname, *loc;
	Props		*p = NULL;
	CSA_session_handle	c_handle;
	CSA_entry_handle	*list;
	CSA_calendar_user	csa_user;
	CSA_return_code		stat;
#ifdef FNS
	char		buf[MAXNAMELEN];
#endif

#ifdef __osf__
        svc_init();
#endif
  
	init_time();
	setlocale(LC_ALL, "");
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

	uname = cm_target2name(target_caladdr);
	loc = cm_target2location(target_caladdr);

	/* check to see if the file name parameter makes sense */

	if (cm_file[0]) {
		if (Mode == archive) {
			if ((output_stream = fopen(cm_file, "w")) == NULL) {
				fprintf(stderr, catgets(DT_catd, 1, 698, "\nCould not open output file \"%s\"\n"), cm_file);
				exit(1);
			}

		}

		if (Mode == restore) {
			if ((input_stream = fopen(cm_file, "r")) == NULL) {
				fprintf(stderr, catgets(DT_catd, 1, 699, "\nCould not open input file \"%s\"\n"), cm_file);
				exit(1);
			}
		}
	}
	csa_user.user_name = target_caladdr;
	csa_user.user_type = 0;
	csa_user.calendar_user_extensions = NULL;
	csa_user.calendar_address = target_caladdr;
	stat = csa_logon(NULL, &csa_user, NULL, NULL, NULL, &c_handle, NULL);

	if (stat != CSA_SUCCESS) {
	  	char *format = XtNewString(catgets(DT_catd, 1, 208, 
					 "\nCould not open calendar \"%s\"\n"));
		fprintf(stderr, format, 
			target_calname ? target_calname : catgets(DT_catd, 1, 209, "UNKNOWN"));
		XtFree(format);
		free(uname);
		free(loc);
		exit(1);
	}

	csa_logoff(c_handle, NULL);
	props_clean_up(p);
	free(p);
	free(uname);
	free(loc);
	return 0;
}
