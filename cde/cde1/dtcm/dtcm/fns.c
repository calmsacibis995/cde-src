/*******************************************************************************
**
**  fns.c
**
**  $XConsortium: fns.c /main/cde1_maint/1 1995/07/14 23:09:37 drk $
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

#pragma ident "@(#)fns.c	1.8 97/01/31 Sun Microsystems, Inc."

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved
 */

#ifdef FNS

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>	/* REQUIRES PTHREADS! */

#include "debug.h"
#include "calendar.h"
#include "props.h"
#include "misc.h"
#include "dtfns.h"
#include "cmfns.h"

/* Data used by registration thread */
typedef struct _Registration_data  {
	char	*username;
	char	*location;
} Registration_data;

static void *registration_thr(void *data);

static int  register_calendar(const char *username, const char *location);

/*
 * Register a calendar location in FNS.
 */
int
cm_register_calendar(const char *username, const char *location)

{
	int error;
	Registration_data	*data;

	data = (Registration_data *)malloc(sizeof(Registration_data));

	if (data == NULL) {
		return -1;
	}

	data->username = strdup(username);
	data->location = strdup(location);

	error = pthread_create(NULL, NULL, registration_thr, data);
	/*	error = thr_create(NULL, 0, registration_thr, data, THR_DETACHED, NULL);*/

	if (error > 0) {
		printf("Could not create registration thread\n");
		return -1;
	} else {
		return 0;
	}
}

static void *
registration_thr(void *data)

{
	Registration_data *reg_data; 

	DP(("registration_thr: THREAD ENTER\n"));
	if (data == NULL) {
		return (void *)-1;
	}

	reg_data = (Registration_data *)data;

	DP(("registration_thr: Registering calendar\n"));
	cmfns_register_calendar(reg_data->username, reg_data->location);

	DP(("registration_thr: Registration complete. Freeing data\n"));
	free(reg_data->username);
	free(reg_data->location);
	free(reg_data);
	DP(("registration_thr: THREAD EXIT\n"));
	return (void *)1;
}

#ifdef FNS_DEMO

/*
 * Extract the FNS name out of a file or directory. In the case of a
 * file it should have the format of
 *
 *	# Zero or more
 *	# comments
 *	fns/name/for/the/fns/object
 *
 * In the case of a directory this routine looks for a file called
 * ".FNSName" in the directory specified.  This file should have the
 * format described above.
 *
 * Caller is responsible for providing the buffer to place the name in.
 *
 *	Returns
 *		-1	Internal error or corrupted file
 *		0	Could not access file/directory
 *		1	Success.  'name' contains the FNS name
 */
int
cmfns_name_from_file(
	const char	*path,	/* Path to file to get name from */
	char		*name,	/* Buffer to put name in */
	int		len)	/* Length of buffer */

{
	int	fd;
	FILE	*fp;
	char	*p;
	struct stat	statb;

	if ((fd = open(path, O_RDONLY)) < 0) {
		return 0;
	}

	if (fstat(fd, &statb) < 0) {
		close(fd);
		return 0;
	}

	if (S_ISDIR(statb.st_mode)) {
		char	*newpath;

		/* path points to a directory.  Look for .FNSName file */
		close(fd);
		newpath = (char *)malloc(strlen(path) + strlen(FNS_FILE) + 2);
		if (newpath == NULL) {
			return -1;
		}
		sprintf(newpath, "%s/%s", path, FNS_FILE);
		if ((fp = fopen(newpath, "r")) == NULL) {
			free(newpath);
			return 0;
		}
		free(newpath);
	} else {
		if ((fp = fdopen(fd, "r")) == NULL) {
			return 0;
		}
	}

	do {
		/* Get line from file */
		if (fgets(name, len, fp) == NULL) {
			fclose(fp);
			return -1;
		}

		/* Make sure we got a full line */
		p = strrchr(name, '\n');

		/*
		 * If it is a comment line or we got a partial line
		 * try again
		 */
	} while (*name == '#' || p == NULL);

	/* Should have the name.  Wipe out newline */
	*p = '\0';

	fclose(fp);
	return 1;
}
#endif /* FNS_DEMO */

#endif /* FNS */
