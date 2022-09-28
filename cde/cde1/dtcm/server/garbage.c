/* $XConsortium: garbage.c /main/cde1_maint/2 1995/09/06 08:25:54 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)garbage.c	1.11 96/11/19 Sun Microsystems, Inc."

#include	<stdio.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<time.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <errno.h>
#include	"log.h"
#include	"rtable4.h"
#include	"cm.h"
#include	"cmscalendar.h"
#include	"tree.h"
#include	"garbage.h"

extern char *pgname;

static CSA_return_code dump_error;
static int fd;

/*
 * forward declaration of functions used within this file
 */
static boolean_t magic_time(time_t t);
static boolean_t visit1(caddr_t node, caddr_t d);
static boolean_t visit2(caddr_t node, caddr_t d);
static void print_file_error(char *file, char *msg);

extern void
_DtCmsCollectOne(_DtCmsCalendar *cal)
{
	Rb_Status	status;
	CSA_return_code csastat;
	char		*bak, *temp, *clog;
	struct stat     sbuf;   /* Check block size and mod time of log. */

	if (cal == NULL)
		return;

	status	= rb_check_tree(APPT_TREE(cal));
	clog	= _DtCmsGetLogFN(cal->calendar);
	temp	= _DtCmsGetTmpFN(cal->calendar);
	bak	= _DtCmsGetBakFN(cal->calendar);

	/* First check if the calendar was modified by something other
	 * than the rpc.cmsd, e.g., manual editing.
	 */
	if (stat(clog, &sbuf) != 0) {
		fprintf(stderr, "%s: cannot acquire files to execute garbage collection.\n", pgname);
		fprintf(stderr, "possible causes: cannot find home directory.\n");
		fprintf(stderr, "\tNIS or your host server might be down.\n");
		fprintf(stderr, "damage: none\n\n");
		goto cleanup;
	}

	/* If modtime or blktime from stat() call is different from
	 * that of the one kept in the cal structure, then something
	 * else modified the log file - do not go on with clean-up, because
	 * the file might be corrupt!
	 */
	if (sbuf.st_mtime != cal->modtime || sbuf.st_blksize != cal->blksize) {
		fprintf(stderr, "%s: cannot acquire files to execute garbage collection.\n", pgname);
		fprintf(stderr, "possible causes: file has been modified, but not through rpc.cmsd.\n");
		fprintf(stderr, "damage: none from rpc.cmsd, but current state unknown\n");
		fprintf(stderr, "Please look for previous backup calendar files in the spool directory\n");
		fprintf(stderr, "(typically, /var/spool/calendar/).\n\n");
		fprintf(stderr, "Please kill the rpc.cmsd daemon, and restart your calendar to determine\nif your calendar is okay.\n");
		goto cleanup;
	}

	/* Now proceed with the garbage collection. */
	if (status != rb_ok || clog==NULL || temp==NULL || bak==NULL) {
		fprintf(stderr, "%s: cannot acquire files to execute garbage collection.\n", pgname);
		fprintf(stderr, "possible causes: cannot find home directory.\n");
		fprintf(stderr, "\tNIS or your host server might be down.\n");
		fprintf(stderr, "damage: none\n\n");
		goto cleanup;
	}

	/* Make sure that the temp file does not exist before garbage collect */
	unlink (temp);
	if (cal->fversion == _DtCMS_VERSION1)
		csastat = _DtCmsCreateLogV1(cal->owner, temp);
	else
		csastat = _DtCmsCreateLogV2(cal->owner, temp);

	if (csastat != CSA_SUCCESS) {
		if (csastat == CSA_E_NO_AUTHORITY) {
			fprintf(stderr, "%s: file error on %s during garbage collection\n",
				pgname, temp);
			fprintf(stderr, "Reason: getpwnam_r() failed. %s%s\n",
				"No passwd entry for owner of ",
				cal->calendar);

			fprintf(stderr, "damage: none\n\n");
		} else {
			print_file_error(temp,
				"file error during garbage collection");
		}
		goto cleanup;
	}

	if (cal->fversion == _DtCMS_VERSION1)
		csastat = _DtCmsDumpDataV1(temp, cal);
	else
		csastat = _DtCmsDumpDataV2(temp, cal);

	if (csastat != CSA_SUCCESS) {
		print_file_error(temp,
			(csastat == (CSA_X_DT_E_BACKING_STORE_PROBLEM) ?
			 "file error during garbage collection" :
			 "can't dump data structure to file during garbage collection"));
		goto cleanup;
	}

	/* mv -f .callog .calbak; mv -f temp .callog */
	if (rename (clog, bak) < 0) {
		perror ("rpc.cmsd: Can't backup callog to .calbak.\nreason:");
		goto cleanup;
	}

	if (rename (temp, clog) < 0) {
		perror("rpc.cmsd: Can't move .caltemp to callog.\nreason:");
		fprintf(stderr, "%s: you may recover %s from %s.\n", pgname,
			clog, bak);
	}

	/* Now get the new callog file's modtime and blksize, so that later,
	 * we can insert/delete/change calendar attributes and appointments.
	 */
	if (stat(clog, &sbuf) != 0) {
		fprintf(stderr, "%s: cannot acquire cleaned-up log files.\n", pgname);
		fprintf(stderr, "possible causes: cannot find home directory.\n");
		fprintf(stderr, "\tNIS or your host server might be down.\n");
		fprintf(stderr, "damage: unknown\n\n");
		goto cleanup;
	}

	/* Assign the modtime and blksize to the calendar structure. */
	cal->modtime=sbuf.st_mtime;
	cal->blksize=sbuf.st_blksize;

cleanup:
	if (bak != NULL) {
		free(bak);
		bak = NULL;
	}
	if (temp != NULL) {
		free(temp);
		temp = NULL;
	}
	if (clog != NULL) {
		free(clog);
		clog = NULL;
	}
}

extern CSA_return_code
_DtCmsDumpDataV1(char *file, _DtCmsCalendar *cal)
{
	CSA_return_code	stat;

	dump_error = CSA_SUCCESS;

	/* Keep the temp log file open during garbage collection. */
	if ((fd = open(file, O_WRONLY | O_APPEND | O_SYNC)) < 0)
	{
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	if ((stat = _DtCmsAppendAccessByFD(fd, cal->alist)) != CSA_SUCCESS)
		return (stat);

	_DtCmsEnumerateUp(cal, visit1);	/* dump the tree */

	if (close(fd) == EOF)
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);

	return (dump_error);
}

extern CSA_return_code
_DtCmsDumpDataV2(char *file, _DtCmsCalendar *cal)
{
	CSA_return_code	stat;

	dump_error = CSA_SUCCESS;

	/* Keep the temp log file open during garbage collection. */
	if ((fd = open(file, O_WRONLY | O_APPEND | O_SYNC)) < 0)
	{
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	/* dump calendar attributes */
	if (cal->num_attrs > 0) {
		if ((stat = _DtCmsAppendCalAttrsByFD(fd, cal->num_attrs,
		    cal->attrs)) != CSA_SUCCESS)
			return (stat);
	}

	if ((stat = _DtCmsAppendHTableByFD(fd, cal->entry_tbl->size,
	    cal->entry_tbl->names, cal->types)) != CSA_SUCCESS)
		return (stat);

	_DtCmsEnumerateUp(cal, visit2);	/* dump the tree */

	if (close(fd) == EOF)
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);

	return (dump_error);
}

static boolean_t
magic_time(time_t t)
{
        boolean_t magic = _B_FALSE;

	/* allocate buffer for mt-safe time */
        struct tm *tm=(struct tm *)malloc(sizeof(tm));

        localtime_r(&t, tm);

        if (tm->tm_hour == 3 && tm->tm_min == 41)
                 magic = _B_TRUE;
	free(tm);

        return(magic);
}

static boolean_t
visit1(caddr_t node, caddr_t d)
{
	boolean_t stop = _B_FALSE;

	switch (((Appt_4 *) d)->tag->tag)
	{
	case otherTag_4:
		/*
		 * otherTags = events read in from files.
		 * Don't write to log.
		 */
		return(stop);
/* No longer needed. */
	}

	if ((dump_error = _DtCmsAppendAppt4ByFD(fd, (Appt_4 *)d, _DtCmsLogAdd)) != CSA_SUCCESS)
        	stop = _B_TRUE;

	return(stop);
}

static boolean_t
visit2(caddr_t node, caddr_t d)
{
	boolean_t stop = _B_FALSE;

	if ((dump_error = _DtCmsAppendEntryByFD(fd, (cms_entry *)d,
	    _DtCmsLogAdd)) != CSA_SUCCESS)
        	stop = _B_TRUE;

	return(stop);
}

static void
print_file_error(char *file, char *msg)
{
	if (file)
		fprintf(stderr, "%s: (%s)%s\n", pgname, file, msg);
	else
		fprintf(stderr, "%s: %s\n", pgname, msg);
	fprintf(stderr, "possible causes: %s, %s, %s, %s\n",
		"host server is down", "disk is full", "out of memory",
		"file protections have changed.");
	fprintf(stderr, "damage: none\n\n");
}


