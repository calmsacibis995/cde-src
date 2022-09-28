/* $XConsortium: log.c /main/cde1_maint/2 1995/09/06 08:26:32 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)log.c	1.28 97/02/10 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#if defined(sun) && defined(_XOPEN_SOURCE)
#include <time.h>
#endif
#include "cm.h"
#include "rtable4.h"
#include "log.h"
#include "iso8601.h"

extern int	daemon_gid, daemon_uid;
extern char	*pgname;
extern int	debug;
static char	*spool_dir = _DtCMS_DEFAULT_DIR;

char *tag_string[] = {
	"0:boolean",
	"1:enum",
	"2:flags",
	"3:sint32",
	"4:uint32",
	"5:string",
	"6:user",
	"7:date_time",
	"8:date_time_range",
	"9:time_duration",
	"10:access_list",
	"11:attendees",
	"12:date_time_list",
	"13:reminder",
	"14:opaque_data",
	NULL
};

/*
 * forward declaration of functions used within this file
 */
static char * cr_to_str(char *s, int size);
static void periodtostr(Interval_4 i, char *q);
static void privacytostr(Privacy_Level_4 p, char *q);
static void apptstatustostr(Appt_Status_4 p, char *q);
static void apptstatustostr(Appt_Status_4 p, char *q);
static void tagstostr(Event_Type_4 p, char *q);
static char * get_fname (char *dir, char *fname, char *who);
static CSA_return_code append_log(int f, char *buf);
static CSA_return_code create_log(char *owner, char *file, int version);
static char *attrs_to_attrliststr(int len, cms_attribute *attrs,
				boolean_t write_hash, boolean_t entryattrs);
static char *grow_n_concat(char *base, char *newstr, int newcount);
static char *get_access_list_string(cms_access_entry *list);
static char *get_date_time_list_string(CSA_date_time_list list);
static CSA_return_code _AppendOneAccessByFD(int f, int type, Access_Entry_4 *p);


/*
 * extern functions
 */

extern char *
_DtCmsGetLogFN(char *who)
{
	return (get_fname (spool_dir, _DtCMS_DEFAULT_LOG, who));
}
 
extern char *
_DtCmsGetBakFN(char *who)
{
	return (get_fname (spool_dir, _DtCMS_DEFAULT_BAK, who));
}
 
extern char *
_DtCmsGetTmpFN(char *who)
{
	return (get_fname (spool_dir, _DtCMS_DEFAULT_TMP, who));
}

extern char *
_DtCmsGetDelFN(char *who)
{
	return (get_fname (spool_dir, _DtCMS_DEFAULT_DEL, who));
}

extern CSA_return_code
_DtCmsCreateLogV1(char *owner, char *file)
{
	return (create_log(owner, file, _DtCMS_VERSION1));
}


extern CSA_return_code
_DtCmsVerifyLogFileInfo(char *calendar, time_t modtime, long blksize)
{
	char        *log;	/* callog file name */
	struct stat  sbuf;	/* buffer for stat() */
	int	     sres;      /* result of stat() call */


	/* Get the callog file name, and then stat() the file. */
	if ((log = _DtCmsGetLogFN(calendar)) == NULL) {
		if (debug)
			fprintf(stderr, "%s: _DtCmsVerifyLogFileInfo(): failed to get callog filename for calendar %s\n",
				pgname, calendar);
		return(CSA_E_INSUFFICIENT_MEMORY);
	}
	sres = stat(log, &sbuf);
	free(log);
	if (sres < 0) {
		perror(pgname);
		return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}


	/* If the current modtime and blksize are different than when
	 * rpc.cmsd initially read in the values, then assume that
	 * the callog file was modified by something else.  Return
	 * with CSA_X_DT_E_BACKING_STORE_PROBLEM so that the write does
	 * not continue, in order to insure that data written out won't
	 * corrupt the callog file.
	 */
	if (modtime == sbuf.st_mtime && blksize == sbuf.st_blksize)
		return(CSA_SUCCESS);
	else {
		if (debug) {
		  	char cbuf[26], cbuf2[26];

		  	ctime_r(&modtime, cbuf);
			ctime_r(&sbuf.st_mtime, cbuf2);
			fprintf(stderr, "%s: _DtCmsVerifyLogFileInfo(): Verification of log file info failed\n", pgname);
			fprintf(stderr, "\tmodtime: initial - %s\t current - %s\n",
				cbuf, cbuf2);
			fprintf(stderr, "\tblksize: initial - %d\t current - %d\n",
				blksize, sbuf.st_blksize);
		}
		return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}
}


extern CSA_return_code
_DtCmsUpdateLogFileInfo(char *calendar, time_t *modtime, long *blksize)
{
	char        *log;	/* callog file name */
	struct stat  sbuf;	/* buffer for stat() */
	int	     sres;      /* result of stat() call */


	/* Get the callog file name, and then stat() the file. */
	if ((log = _DtCmsGetLogFN(calendar)) == NULL) {
		if (debug)
			fprintf(stderr, "%s: failed to get callog filename for calendar %s\n",
				pgname, calendar);
		return(CSA_E_INSUFFICIENT_MEMORY);
	}
	sres = stat(log, &sbuf);
	free(log);
	if (sres < 0) {
		perror(pgname);
		return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}


	/* Update the modification time and blksize info. */
	*modtime = sbuf.st_mtime;
	*blksize = sbuf.st_blksize;
#ifdef UNDEF
	if (debug) {
		char cbuf[26];

		ctime_r(modtime, cbuf);
		fprintf(stderr, "Log file info modified:\n\tmodtime: %s\n",
			cbuf);
		fprintf(stderr, "\tblksize: %d\n", (*blksize));
	}
#endif
	return(CSA_SUCCESS);
}


extern CSA_return_code
_DtCmsAppendHTableByFN(char *file, uint size, char **names, int *types)
{
	int	f;
	CSA_return_code	stat;

	if ((f = open(file, O_WRONLY | O_APPEND | O_SYNC)) < 0) {
		perror(pgname);
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	stat = _DtCmsAppendHTableByFD(f, size, names, types);

	close(f);
	return (stat);
}

/*
 * the first element of the arrays is not used
 */
extern CSA_return_code
_DtCmsAppendHTableByFD(int fd, uint size, char **names, int *types)
{
	CSA_return_code	stat;
	char	*tptr, *buf, tmpbuf[BUFSIZ/4];
	int	i, tcount, count = 15;	/* strlen("(entrytable )\n") + 1 */

	if ((buf = malloc(BUFSIZ)) == NULL) return (CSA_E_INSUFFICIENT_MEMORY);
	tcount = BUFSIZ;
	strcpy(buf, "(entrytable ");

	for (i = 1; i <= size; i++) {
		sprintf(tmpbuf, "(%d \"%s\" \"%s\")\n", i, names[i],
			tag_string[types[i]]);

		count += strlen(tmpbuf);
		if (tcount < count) {
			if ((tptr = grow_n_concat(buf, tmpbuf, tcount + BUFSIZ))
			    == NULL) {
				free(buf);
				return (CSA_E_INSUFFICIENT_MEMORY);
			}
			buf = tptr;
			tcount += BUFSIZ;
		} else
			strcat(buf, tmpbuf);
	}

	strcat(buf, ")\n");

	stat = append_log(fd, buf);

	free(buf);
	return (stat);
}

extern CSA_return_code
_DtCmsAppendEntryByFN(char *file, cms_entry *entry, _DtCmsLogOps op)
{
	int	f;
	CSA_return_code	stat;

	if ((f = open(file, O_WRONLY | O_APPEND | O_SYNC)) < 0) {
		perror(pgname);
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	stat = _DtCmsAppendEntryByFD(f, entry, op);

	close(f);
	return (stat);
}

extern CSA_return_code
_DtCmsAppendEntryByFD(int f, cms_entry *entry, _DtCmsLogOps op)
{
	CSA_return_code	stat;
	int	count;
	char	*cptr, *tptr, *buf;
	char	isotime[25];

	if ((buf = malloc(100)) == NULL) return (CSA_E_INSUFFICIENT_MEMORY);

	_csa_tick_to_iso8601(entry->key.time, isotime);
	sprintf(buf, "(%s \"%s\" key: %d%s",
		(op == _DtCmsLogAdd ? "add" : "remove"),
		isotime, entry->key.id,
		(op == _DtCmsLogAdd ? "\nhashedattributes: (" : ""));
	count = strlen(buf) + 4;

	if (op == _DtCmsLogAdd && entry->num_attrs > 0) {
		if ((cptr = attrs_to_attrliststr(entry->num_attrs,
		    entry->attrs, _B_TRUE, _B_TRUE)) == NULL) {
			free(buf);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		count += strlen(cptr);
		if ((tptr = grow_n_concat(buf, cptr, count)) == NULL) {
			free(cptr);
			free(buf);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
		free(cptr);
		buf = tptr;
	}

	/* closing for attr list and closing for entry */
	if (op == _DtCmsLogAdd)
		strcat(buf, "))\n");
	else
		strcat(buf, ")\n");

	stat = append_log(f, buf);

	free(buf);
	return (stat);
}

extern CSA_return_code
_DtCmsAppendCalAttrsByFN(char *file, int size, cms_attribute * attrs)
{
	int	f;
	CSA_return_code	stat;

	if (file == NULL || size <= 0)
		return (CSA_E_INVALID_PARAMETER);

	if ((f = open(file, O_WRONLY | O_APPEND | O_SYNC)) < 0) {
		perror(pgname);
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	stat = _DtCmsAppendCalAttrsByFD(f, size, attrs);

	close(f);
	return (stat);
}

extern CSA_return_code
_DtCmsAppendCalAttrsByFD(int f, int size, cms_attribute * attrs)
{
	CSA_return_code	stat;
	char	*buf, *attrstr;
	char	*prefix = "(calendarattributes ";
	char	*subfix = ")\n";
	int	count;

	if ((attrstr = attrs_to_attrliststr(size, attrs, _B_FALSE, _B_FALSE))
	    == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	count = strlen(prefix) + strlen(attrstr) + strlen(subfix) + 1;
	if ((buf = malloc(count)) == NULL) {
		free(attrstr);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}
	sprintf(buf, "%s%s%s", prefix, attrstr, subfix);

	stat = append_log(f, buf);

	free(attrstr);
	free(buf);
	return (stat);
}

extern CSA_return_code
_DtCmsAppendAppt4ByFN(char *file, Appt_4 *appt, _DtCmsLogOps op)
{
	int	f;
	CSA_return_code	stat;

	if ((f = open(file, O_WRONLY | O_APPEND | O_SYNC)) < 0) {
		perror(pgname);
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	stat = _DtCmsAppendAppt4ByFD(f, appt, op);

	close(f);
	return (stat);
}

extern CSA_return_code
_DtCmsAppendAppt4ByFD(int f, Appt_4 *appt, _DtCmsLogOps op)
{
        /* cbuf is buffer for use with ctime_r */
        char cbuf[26];
	char *cptr;
	char tmpbuf[BUFSIZ];
	char buf[BUFSIZ*3], buf2[BUFSIZ*2]; /* 1 BUFSIZ for what fields, 
					       1 BUFSIZ for mailto field,
					       and 1 BUFSIZ for the rest */

	ctime_r(&appt->appt_id.tick, cbuf);
	cbuf[24] = NULL;		/* strip off CR */

	buf[0] = NULL;
	switch (op) {
	case _DtCmsLogAdd:
		sprintf(buf, "(add \"%s\" key: %d ", cbuf, appt->appt_id.key);
		if (appt->what) {
			cptr = cr_to_str(appt->what, strlen(appt->what));
			if (cptr != NULL) {
				sprintf(buf2, "what: \"%s\" ", cptr);
				strcat(buf, buf2);
				free(cptr);
			} else {
				return (CSA_E_INSUFFICIENT_MEMORY);
			}
		}
		if (appt->client_data) {
			sprintf(buf2, "details: \"%s\" ", appt->client_data);
			strcat(buf, buf2);
		}
		if (appt->duration) {
			sprintf(buf2, "duration: %d ", appt->duration);
			strcat(buf, buf2);
		}

		periodtostr (appt->period.period, tmpbuf);
		sprintf(buf2, "period: %s ", tmpbuf);
		strcat(buf, buf2);

		if (appt->period.nth != 0) {
			sprintf (buf2, "nth: %d ", appt->period.nth);
			strcat(buf, buf2);
		}
		if (appt->period.enddate != 0) {
			ctime_r(&(appt->period.enddate), cbuf);
			cbuf[24] = NULL; /* strip off CR */
			sprintf(buf2, "enddate: \"%s\" ", cbuf);
			strcat(buf, buf2);
		}

		sprintf(buf2, "ntimes: %d ", appt->ntimes);
		strcat(buf, buf2);

		if (appt->exception != NULL) {
			struct Except_4 *e = appt->exception;
			strcat(buf, "exceptions: (");
			while(e != NULL) {
				sprintf(buf2, "%d ", e->ordinal);
				strcat(buf, buf2);
				e = e->next;
			}
			strcat(buf, ") ");
		}

		if (appt->author != NULL) {
			sprintf(buf2, "author: \"%s\" ", appt->author);
			strcat(buf, buf2);
		}
		if (appt->attr != NULL) {
			struct Attribute_4 *item = appt->attr;
			strcat(buf, "attributes: (");
			while(item != NULL) {
				sprintf(buf2, "(\"%s\",\"%s\",\"%s\")",
					item->attr, item->value,
					item->clientdata);
				strcat(buf, buf2);
				item = item->next;
			}
			strcat(buf, ") ");
		}
		if (appt->tag != NULL) {
			struct Tag_4 *item = appt->tag;
			strcat(buf, "tags: (");
			while(item != NULL) {
				tagstostr(item->tag, tmpbuf);
				sprintf(buf2, "(%s , %d)", tmpbuf,
					item->showtime);
				strcat(buf, buf2);
				item = item->next;
			}
			strcat(buf, ") ");
		}

		apptstatustostr(appt->appt_status, tmpbuf);
		sprintf(buf2, "apptstat: %s ", tmpbuf);
		strcat(buf, buf2);

		privacytostr(appt->privacy, tmpbuf);
		sprintf(buf2, "privacy: %s )\n", tmpbuf);
		strcat(buf, buf2);

		break;    

	case _DtCmsLogRemove:
		sprintf(buf, "(remove \"%s\" key: %d)\n", cbuf,
			appt->appt_id.key);
		break;
	}

	return (append_log(f, buf));
}

extern CSA_return_code
_DtCmsAppendAccessByFN(char *file, Access_Entry_4 *p)
{
	int	f;
	CSA_return_code	stat;

	if ((f = open(file, O_WRONLY | O_APPEND | O_SYNC)) < 0) {
		perror(pgname);
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	stat = _DtCmsAppendAccessByFD(f, p);

	close(f);
	return (stat);
}

extern CSA_return_code
_DtCmsAppendAccessByFD(int f, Access_Entry_4 *p)
{
	Access_Entry_4 *ptr, *head;
	CSA_return_code	stat;

	/* reverse the list order */
	head = NULL;
	while (p != NULL) {
		ptr = p->next;
		p->next = head;
		head = p;
		p = ptr;
	}

	if ((stat = _AppendOneAccessByFD(f, access_read_4, head))
	    != CSA_SUCCESS)
		goto done;

	if ((stat = _AppendOneAccessByFD(f, access_write_4, head))
	    != CSA_SUCCESS)
		goto done;

	if ((stat = _AppendOneAccessByFD(f, access_delete_4, head))
	    != CSA_SUCCESS)
		goto done;

	stat = _AppendOneAccessByFD(f, access_exec_4, head);

done:
	/* reverse it back */
	p = NULL;
	while (head != NULL) {
		ptr = head->next;
		head->next = p;
		p = head;
		head = ptr;
	} 

	return (stat);
}

extern boolean_t
_DtCmsPrintAppt4(caddr_t data)
{
	Appt_4 *appt = (Appt_4 *)data;
	/* buffer for use in ctime_r */
	char tmbuf[26];
	char *tmstr;
	char buf[BUFSIZ];

	fprintf(stderr, "*** V4 appointement: ***\n\n");

	ctime_r(&appt->appt_id.tick, tmbuf);
	tmbuf[24] = NULL;		/* strip off CR */

	if (fprintf(stderr, "(add \"%s\" ", tmbuf)==EOF) {
		return (_B_TRUE);
	}
	if (fprintf(stderr, "key: %d ", appt->appt_id.key)==EOF) {
		return (_B_TRUE);
	}
	if (appt->what) {
		tmstr = cr_to_str(appt->what, strlen(appt->what));
		if (fprintf(stderr, "what: \"%s\" ", tmstr) == EOF) {
			free(tmstr);
			return (1);
		} else
			free(tmstr);
	}
	if (appt->client_data) {
		if (fprintf(stderr, "details: \"%s\" ", appt->client_data)
		    == EOF)
			return (_B_TRUE);
	}
	if (appt->duration) {
		if (fprintf(stderr, "duration: %d ", appt->duration) == EOF)
			return (_B_TRUE);
	}

	buf[0]=NULL;  
	periodtostr (appt->period.period, buf);
	if (fprintf(stderr, "period: %s ", buf) == EOF)
		return (_B_TRUE);

	if (appt->period.nth != 0) {
		if (fprintf (stderr, "nth: %d ", appt->period.nth) == EOF)
			return (_B_TRUE);
	}
	if (appt->period.enddate != 0) {
		ctime_r(&(appt->period.enddate), tmbuf);
		tmbuf[24] = NULL; /* strip off CR */
		if (fprintf(stderr, "enddate: \"%s\" ", tmbuf) == EOF)
			return (_B_TRUE);
	}

	if (fprintf(stderr, "ntimes: %d ", appt->ntimes) == EOF)
		return (_B_TRUE);

	if (appt->exception != NULL) {
		struct Except_4 *e = appt->exception;
		if (fprintf(stderr, "exceptions: (") == EOF)
			return (_B_TRUE);
		while(e != NULL) {
			if (fprintf(stderr, "%d ", e->ordinal) == EOF)
				return (_B_TRUE);
			e = e->next;
		}
		if (fprintf(stderr, ") ") == EOF)
			return (_B_TRUE);
	}
	if (appt->author != NULL) {
		if (fprintf(stderr, "author: \"%s\" ", appt->author) == EOF)
			return (_B_TRUE);
	}
	if (appt->attr != NULL) {
		struct Attribute_4 *item = appt->attr;
		if (fprintf(stderr, "attributes: (") == EOF)
			return (_B_TRUE);
		while(item != NULL) {
			if (fprintf(stderr, "(\"%s\",\"%s\",\"%s\")",
			    item->attr, item->value, item->clientdata) == EOF)
				return (_B_TRUE);
			item = item->next;
		}
		if (fprintf(stderr, ") ") == EOF)
			return (_B_TRUE);
	}
	if (appt->tag != NULL) {
		struct Tag_4 *item = appt->tag;
		if (fprintf(stderr, "tags: (") == EOF)
			return (_B_TRUE);
		while(item != NULL) {
			buf[0]=NULL;
			tagstostr(item->tag, buf);
			if (fprintf(stderr, "(%s , %d)", buf, item->showtime)
			    == EOF)
				return (_B_TRUE);
			item = item->next;
		}
		if (fprintf(stderr, ") ") == EOF)
			return (_B_TRUE);
	}


	buf[0]=NULL;
	apptstatustostr(appt->appt_status, buf);
	if (fprintf(stderr, "apptstat: %s ", buf) == EOF)
		return (_B_TRUE);

	buf[0]=NULL;
	privacytostr(appt->privacy, buf);
	if (fprintf(stderr, "privacy: %s )\n", buf) == EOF)
		return (_B_TRUE);

	return (_B_FALSE);
}

extern void
_DtCmsPrintExceptions(int len, int *exceptions)
{
	int i;

	fprintf(stderr, "\nexception part:\n");
	fprintf(stderr, "number of exception = %d\n", len);
	fprintf(stderr, "exceptions:");
	for (i = 0; i < len; i++) {
		fprintf(stderr, " %d", exceptions[i]);
	}
	fprintf(stderr, "\n");
}

extern CSA_return_code
_DtCmsCreateLogV2(char *owner, char *file)
{
	return (create_log(owner, file, _DtCMS_VERSION4));
}

extern CSA_return_code
_DtCmsGetFileSize(char *calendar, int *size)
{
	char		*log;
	struct stat	info;

	if ((log = _DtCmsGetLogFN(calendar)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (stat(log, &info) != 0) {
		free(log);
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	} else {
		*size = info.st_size;
		free(log);
		return (CSA_SUCCESS);
	}
}

extern void
_DtCmsTruncateFile(char *calendar, int size)
{
	char	*log;
	int	f;

	if ((log = _DtCmsGetLogFN(calendar)) == NULL)
		return;

	/* truncate log file to specified size */
	if ((f = open(log, O_RDWR | O_APPEND | O_SYNC)) >= 0)
		ftruncate(f, size);

	free(log);
}

extern CSA_return_code
_DtCmsRemoveLog(char *calendar, char *user)
{
	CSA_return_code	stat = CSA_SUCCESS;
	char		*log, *dlog;

	if ((log = _DtCmsGetLogFN(calendar)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if ((dlog = _DtCmsGetDelFN(calendar)) == NULL) {
		free(log);
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	if (rename(log, dlog) < 0)
		stat = CSA_X_DT_E_BACKING_STORE_PROBLEM;

	free(log);
	free(dlog);

	return (stat);
}

extern CSA_return_code
_DtCmsWriteVersionString(char *file, int version)
{
	struct	tm *tm=(struct tm *)malloc(sizeof(struct tm));
	time_t	tmval;
	char	tmstr[26];
	int	fd, len;
	char	buf[BUFSIZ];
	CSA_return_code stat = CSA_SUCCESS;

	tmval = time((time_t *) 0);
	localtime_r(&tmval, tm);
	asctime_r(tm, tmstr);
	free(tm);
	tmstr[24] = NULL;		/* strip off CR */

	if ((fd = open(file, O_WRONLY|O_TRUNC|O_SYNC)) < 0) {
		if (debug)
			fprintf(stderr, "%s: failed to open %s in %s\n",
				pgname, file, _DtCmsWriteVersionString);
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	sprintf(buf, "Version: %d\n**** start of log on %s ****\n\n",
		version, tmstr);

	len = strlen(buf);
	if (write(fd, buf, len) != len) {
		perror(pgname);
		stat = CSA_X_DT_E_BACKING_STORE_PROBLEM;
	}
	close(fd);

	return (stat);
}

/*
 * if everything works fine, return 0; otherwise, return -1
 */
extern int
_DtCmsSetFileMode(
	char *file,
	int fd,
	int uid,
	int gid,
	mode_t mode,
	boolean_t changeeuid,
	boolean_t printerr)
{
	int	error;
	char	buff[BUFSIZ];
	int	saveuid;
	int	res = 0;

	if (changeeuid == _B_TRUE) {
#ifndef AIX
#ifdef HPUX
		setuid (0);
#else
		saveuid = geteuid();
		seteuid (0);
#endif
#endif
	}

	if (fchmod (fd, mode) < 0) {
		if (printerr == _B_TRUE) {
			error = errno;
			sprintf (buff, "%s: chmod %s to %o failed.\n%s: System error",
				pgname, file, mode, pgname);
			errno = error;
			perror (buff);
		}
		res = -1;
	} else if (fchown (fd, uid, gid) < 0) {
		if (printerr == _B_TRUE) {
			error = errno;
			sprintf (buff, "%s: chown %s to (uid=%d,gid=%d) failed.\n%s%s",
				pgname, file, uid, gid, pgname, ": System error");
			errno = error;
			perror (buff);
		}
		res = -1;
	}

#ifndef AIX
#ifdef HPUX
	if (changeeuid == _B_TRUE && setuid (daemon_uid) < 0)
#else
	if (changeeuid == _B_TRUE && seteuid (saveuid) < 0)
#endif
	{
		if (printerr == _B_TRUE) {
			error = errno;
			sprintf (buff, "%s: Can't switch process uid back to daemon.\n%s%s",
				pgname, pgname, ": System error");
			errno = error;
			perror (buff);
		}
	}
#endif /* AIX */

	return(res);
}

static void
periodtostr(Interval_4 i, char *q)
{
	if (q==NULL) return;
	q[0]=NULL;
	switch (i) {
		case single_4:
			strcpy (q, "single");
			break;
		case daily_4:
			strcpy (q, "daily");
			break;
		case weekly_4:
			strcpy (q, "weekly");
			break;
		case biweekly_4:
			strcpy (q, "biweekly");
			break;
		case monthly_4:
			strcpy (q, "monthly");
			break;
		case yearly_4:
			strcpy (q, "yearly");
			break;
		case nthWeekday_4:
			strcpy (q, "nthWeekday");
			break;
		case everyNthDay_4:
			strcpy (q, "everyNthDay");
			break;
		case everyNthWeek_4:
			strcpy (q, "everyNthWeek");
			break;
		case everyNthMonth_4:
			strcpy (q, "everyNthMonth");
			break;
		case monThruFri_4:
			strcpy (q, "monThruFri");
			break;
		case monWedFri_4:
			strcpy (q, "monWedFri");
			break;
		case tueThur_4:
			strcpy (q, "tueThur");
			break;
		case daysOfWeek_4:
			strcpy (q, "daysOfWeek");
			break;
		default:
			strcpy (q, "single");
			break;
	}
}

static void
privacytostr(Privacy_Level_4 p, char *q)
{
	if (q==NULL) return;
	q[0]=NULL;
	switch(p) {
	case public_4:
		strcpy(q, "public");
		break;
	case private_4:
		strcpy(q, "private");
		break;
	case semiprivate_4:
		strcpy(q, "semiprivate");
		break;
	default:
		strcpy(q, "public");
		break;
	}
}

static void
apptstatustostr(Appt_Status_4 p, char *q)
{
	if (q==NULL) return;
	q[0]=NULL;
	switch(p) {
	case active_4:
		strcpy(q, "active");
		break;
	case pendingAdd_4:
		strcpy(q, "pendingAdd");
		break;
	case pendingDelete_4:
		strcpy(q, "pendingDelete");
		break;
	case committed_4:
		strcpy(q, "committed");
		break;
	case cancelled_4:
		strcpy(q, "cancelled");
		break;
	case completed_4:
		strcpy(q, "completed");
		break;
	default:
		strcpy(q, "active");
		break;
	}
}

static void
tagstostr(Event_Type_4 p, char *q)
{
	if (q==NULL) return;
	q[0]=NULL;
	switch(p) {
	case appointment_4:
		strcpy(q, "appointment");
		break;
	case reminder_4:
		strcpy(q, "reminder");
		break;
	case otherTag_4:
		strcpy(q, "otherTag");
		break;
	case holiday_4:
		strcpy(q, "holiday");
		break;
	case toDo_4:
		strcpy(q, "toDo");
		break;
	default:
		strcpy(q, "appointment");
		break;
	}
}

static char *
get_fname (char *dir, char *fname, char *who)
{
	char *buf;

	buf = (char *)malloc(strlen(dir) + strlen(fname) + strlen(who) + 3);
	if (buf != NULL)
		sprintf (buf, "%s/%s.%s", dir, fname, who);
        return (buf);
}

static CSA_return_code
create_log(char *owner, char *file, int version)
{
	int	uid;
	int	fd;
	char	*ptr;
	CSA_return_code stat;
	struct	passwd *pw;
#ifdef SunOS
	struct	passwd pwstruct;
	char	buf[BUFSIZ];
	int	status;
#endif

	ptr = strchr(owner, '@');
	if (ptr) {
	  *ptr = NULL;
	}
#ifdef SunOS
	status = getpwnam_r(owner, &pwstruct, buf, BUFSIZ-1, &pw);
	if (status < 0) {
	  pw = NULL;
	}
#else
	pw = getpwnam(owner);
#endif
	if (ptr) {
	  *ptr = '@';
	}
	if (pw == NULL) {
		return(CSA_E_NO_AUTHORITY);
	} else {
		uid = pw->pw_uid;
	}

	/* Read by owner and Read/Write by group (gid must be daemon) */
	fd = open(file, O_WRONLY|O_CREAT|O_EXCL, _DtCMS_DEFAULT_MODE);
	if (fd < 0) {
		if (debug)
			fprintf(stderr, "%s: can't create %s\n", pgname, file);

		if (errno == EEXIST)
			return(CSA_E_CALENDAR_EXISTS);
		else
			return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	if (_DtCmsSetFileMode(file, fd, uid, daemon_gid, _DtCMS_DEFAULT_MODE,
	    _B_TRUE, _B_TRUE) < 0) {
		close(fd);
		unlink(file);
		return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}
	close(fd);

	if ((stat = _DtCmsWriteVersionString(file, version)) != CSA_SUCCESS) {
		unlink(file);
	}

	return (stat);
}

static CSA_return_code
append_log(int f, char *buf)
{
	CSA_return_code status;
	struct stat finfo;
	int file_size = 0, nbytes_written = 0, nbytes_towrite = 0;

	if (buf == NULL)
		return (CSA_E_INVALID_PARAMETER);

	if (fstat(f, &finfo) < 0) {
		perror(pgname);
		return (CSA_X_DT_E_BACKING_STORE_PROBLEM);
	} else
		file_size = finfo.st_size;

	nbytes_towrite = strlen(buf);
	nbytes_written = write(f, buf, nbytes_towrite);
	if (nbytes_written != nbytes_towrite) {
		if (errno == ENOSPC)
			status = CSA_E_DISK_FULL;
		else
			status = CSA_X_DT_E_BACKING_STORE_PROBLEM;

		perror(pgname);
		ftruncate(f, file_size);
		return (status);	
	}

	return(CSA_SUCCESS);
}

static char *
grow_n_concat(char *base, char *newstr, int newcount)
{
	char *ptr;

	if ((ptr = realloc(base, newcount)) == NULL)
		return (NULL);

	if (base)
		strcat(ptr, newstr);
	else
		strcpy(ptr, newstr);

	return (ptr);
}

/*
 * format of attributes:
 * CSA_VALUE_BOOLEAN:		("name","type","number")
 * CSA_VALUE_ENUMERATED:	("name","type","number")
 * CSA_VALUE_FLAGS:		("name","type","number")
 * CSA_VALUE_SINT32:		("name","type","number")
 * CSA_VALUE_UINT32:		("name","type","number")
 * CSA_VALUE_STRING:		("name","type","string")
 * CSA_VALUE_ACCESS_LIST:	("name","type","user:rights [user:rights]")
 * CSA_VALUE_CALENDAR_USER:	("name","type","string")
 * CSA_VALUE_DATE_TIME:		("name","type","datetime")
 * CSA_VALUE_DATE_TIME_RANGE:	("name","type","datetimerange")
 * CSA_VALUE_TIME_DURATION:	("name","type","timeduration")
 * CSA_VALUE_DATE_TIME_LIST:	("name","type","datetime [datetime]")
 * CSA_VALUE_REMINDER:		("name","type","string:number:string") or
 * 				("name","type","string:string:number:number:string")
 * CSA_VALUE_OPAQUE_DATA:	("name","type","number:string")
 *
 * format of attributes (when only hashed number is written):
 * CSA_VALUE_BOOLEAN:		(hash_number "number")
 * CSA_VALUE_ENUMERATED:	(hash_number "number")
 * CSA_VALUE_FLAGS:		(hash_number "number")
 * CSA_VALUE_SINT32:		(hash_number "number")
 * CSA_VALUE_UINT32:		(hash_number "number")
 * CSA_VALUE_STRING:		(hash_number "string")
 * CSA_VALUE_ACCESS_LIST:	(hash_number "user:rights [user:rights]")
 * CSA_VALUE_CALENDAR_USER:	(hash_number "string")
 * CSA_VALUE_DATE_TIME:		(hash_number "datetime")
 * CSA_VALUE_DATE_TIME_RANGE:	(hash_number "datetimerange")
 * CSA_VALUE_TIME_DURATION:	(hash_number "timeduration")
 * CSA_VALUE_DATE_TIME_LIST:	(hash_number "datetime [datetime]")
 * CSA_VALUE_REMINDER:		(hash_number "string:number:string") or
 * 				(hash_number "string:string:number:number:string")
 * CSA_VALUE_OPAQUE_DATA:	(hash_number "number:string")
 *
 * Note: element 0 of the array is not used
 */
static char *
attrs_to_attrliststr(
	int		num_attr,
	cms_attribute	*attrs,
	boolean_t	write_hash,
	boolean_t	entryattrs)
{
	int	i, count, tcount;
	char	*ptr, *buf = NULL;
	char	tmpbuf[BUFSIZ/2], tmpbuf2[BUFSIZ/2], *body;
	CSA_opaque_data *opq;

	if ((buf = malloc(BUFSIZ+1)) == NULL) return (NULL);
	tcount = BUFSIZ;

	for (i = 1, count = 0, *buf = NULL; i <= num_attr; i++) {

		if (attrs[i].value == NULL || (entryattrs &&
		    (i == CSA_ENTRY_ATTR_NUMBER_RECURRENCES_I ||
		    i == CSA_X_DT_ENTRY_ATTR_REPEAT_TYPE_I ||
		    i == CSA_X_DT_ENTRY_ATTR_REPEAT_TIMES_I ||
		    i == CSA_X_DT_ENTRY_ATTR_REPEAT_INTERVAL_I ||
		    i == CSA_X_DT_ENTRY_ATTR_REPEAT_OCCURRENCE_NUM_I ||
		    i == CSA_X_DT_ENTRY_ATTR_SEQUENCE_END_DATE_I)))
			continue;

		if (write_hash)
			/* open_bracket hash_number open quote for value */
			sprintf(tmpbuf, "(%d \"", attrs[i].name.num);
		else
			/* open bracket name, type, open quote for value */
			sprintf(tmpbuf, "(\"%s\",\"%s\",\"", attrs[i].name.name,
				tag_string[attrs[i].value->type]);

		/* value string */
		body = NULL;
		*tmpbuf2 = NULL;
		switch (attrs[i].value->type) {
		case CSA_VALUE_ENUMERATED:
		case CSA_VALUE_SINT32:
			sprintf(tmpbuf2, "%d",
				attrs[i].value->item.sint32_value);
			break;

		case CSA_VALUE_BOOLEAN:
		case CSA_VALUE_FLAGS:
		case CSA_VALUE_UINT32:
			sprintf(tmpbuf2, "%u",
				attrs[i].value->item.uint32_value);
			break;

		case CSA_VALUE_STRING:
		case CSA_VALUE_DATE_TIME:
		case CSA_VALUE_DATE_TIME_RANGE:
		case CSA_VALUE_TIME_DURATION:
		case CSA_VALUE_CALENDAR_USER:
			if (attrs[i].value->item.string_value) {
				if ((ptr = cr_to_str(attrs[i].value->\
				    item.string_value,
				    strlen(attrs[i].value->item.string_value)))
				    == NULL) {
					free(buf);
					return (NULL);
				}
			} else
				ptr = NULL;

			body = ptr;
			break;

		case CSA_VALUE_REMINDER:
			opq = &attrs[i].value->item.reminder_value->\
				reminder_data;
			if (attrs[i].value->item.reminder_value->repeat_count
			    > 1)
			{
			    sprintf(tmpbuf2, "%s:%s:%u:%u:",
				attrs[i].value->item.reminder_value->lead_time,
				(attrs[i].value->item.reminder_value->snooze_time ?
				attrs[i].value->item.reminder_value->snooze_time:""),
				attrs[i].value->item.reminder_value->repeat_count,
				opq->size);
			} else {
			    sprintf(tmpbuf2, "%s:%u:",
				attrs[i].value->item.reminder_value->lead_time,
				opq->size);
			}

			if (opq->size > 0) {
				if ((ptr = cr_to_str((char *)opq->data,
				    opq->size)) == NULL) {
					free(buf);
					return (NULL);
				}
			} else
				ptr = NULL;
			body = ptr;

			break;

		case CSA_VALUE_OPAQUE_DATA:
			opq = attrs[i].value->item.opaque_data_value;
			sprintf(tmpbuf2, "%u:", opq->size);

			if (opq->size > 0) {
				if ((ptr = cr_to_str((char *)opq->data,
				    opq->size)) == NULL) {
					free(buf);
					return (NULL);
				}
			} else
				ptr = NULL;
			body = ptr;
			break;

		case CSA_VALUE_ACCESS_LIST:
			if (attrs[i].value->item.access_list_value) {
				if ((ptr = get_access_list_string(
				    attrs[i].value->item.access_list_value))
				    == NULL) {
					free(buf);
					return (NULL);
				}
			} else
				ptr = NULL;
			body = ptr;
			break;

		case CSA_VALUE_DATE_TIME_LIST:
			if (attrs[i].value->item.date_time_list_value) {
				if ((ptr = get_date_time_list_string(
				    attrs[i].value->item.date_time_list_value))
				    == NULL) {
					free(buf);
					return (NULL);
				}
			} else
				ptr = NULL;
			body = ptr;
			break;
		}

		count += strlen(tmpbuf) + strlen(tmpbuf2) +
				(body ? strlen(body) : 0) + 3; /* closing */
		if (tcount < count) {
			if ((ptr = realloc(buf, tcount+BUFSIZ)) == NULL) {
				if (body) free(body);
				free(buf);
				return NULL;
			}
			buf = ptr;
			tcount += BUFSIZ;
		}
		strcat(buf, tmpbuf);
		if (*tmpbuf2 != NULL) strcat(buf, tmpbuf2);
		if (body) strcat(buf, body);
		strcat(buf, "\")\n");
		if (body) free(body);
	}

	return (buf);
}

static char *
get_access_list_string(cms_access_entry *list)
{
	char *ptr = NULL, *tptr, buf[BUFSIZ/4];
	int count, tcount;

	/* do the first one */
	if ((ptr = malloc(BUFSIZ+1)) == NULL) return (NULL);
	tcount = BUFSIZ;
	sprintf(ptr, "%s:%u", list->user, list->rights);
	count = strlen(ptr);

	while (list->next) {
		list = list->next;
		sprintf(buf, " %s:%u", list->user, list->rights);

		count += strlen(buf);
		if (tcount < count) {
			if ((tptr = grow_n_concat(ptr, buf, tcount+BUFSIZ))
			    == NULL) {
				free(ptr);
				return (NULL);
			}
			ptr = tptr;
			tcount += BUFSIZ;
		} else
			strcat(ptr, buf);
	}
	return (ptr);
}

static char *
cr_to_str(char *s, int size)
{
	int j, k;
	char *newstr;

        if (s==NULL)
		return(NULL);

	if ((newstr = (char *) calloc(1, (unsigned)((2 * size) + 2))) == NULL)
		return (NULL);
	k = 0;
	for (j=0; j<size; j++) {
		if (s[j]=='\n') {
			newstr[k] = '\\';
			newstr[k+1] = 'n';
			k+=2;
		}
		else if (s[j]=='\\') {
			newstr[k] = '\\';
			newstr[k+1] = '\\';
			k+=2;
		}
		else if (s[j]=='\"') {
			newstr[k] = '\\';
			newstr[k+1] = '\"';
			k+=2;
		}
		else if (s[j]=='\r') {
			newstr[k] = '\\';
			newstr[k+1] = '\r';
			k+=2;
		}
		else {
			newstr[k] = s[j];
			k++;
		}
	}
	newstr[k] = NULL;
	return(newstr);
}

static char *
get_date_time_list_string(CSA_date_time_list list)
{
	char *ptr = NULL, *tptr, buf[80];
	int count, datestrlen, tcount;

	/* do the first one */
	sprintf(buf, "%s", list->date_time);
	if ((ptr = strdup(buf)) == NULL)
		return (NULL);

	if ((ptr = malloc(BUFSIZ+1)) == NULL) return (NULL);
	tcount = BUFSIZ;
	strcpy(ptr, list->date_time);
	count = strlen(ptr);

	datestrlen = count + 1;
	while (list->next) {
		list = list->next;
		sprintf(buf, " %s", list->date_time);

		count += datestrlen;
		if (tcount < count) {
			if ((tptr = grow_n_concat(ptr, buf, tcount+BUFSIZ))
			    == NULL) {
				free(ptr);
				return (NULL);
			}
			ptr = tptr;
			tcount += BUFSIZ;
		} else
			strcat(ptr, buf);
	}
	return (ptr);
}

static CSA_return_code
_AppendOneAccessByFD(int f, int type, Access_Entry_4 *p)
{
	CSA_return_code	stat;
	int	count = 12; /* (access  )\n */
	char	*p_type;
	char	*buf;

	if (type == access_read_4)
		p_type = "read";
	else if (type == access_write_4)
		p_type = "write";
	else if (type == access_delete_4)
		p_type = "delete";
	else if (type == access_exec_4)
		p_type = "exec";
	else
		return (CSA_E_INVALID_PARAMETER);

	count = count + strlen(p_type);
	if ((buf = malloc(count)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	sprintf(buf, "(access %s ", p_type);

	while(p != NULL) {
		if (p->who != NULL && (p->access_type & type)) {
			count = count + strlen(p->who) + 3;
			if ((buf = realloc(buf, count)) == NULL)
				return (CSA_E_INSUFFICIENT_MEMORY);
			else {
				strcat(buf, "\"");
				strcat(buf, p->who);
				strcat(buf, "\" ");
			}
		}
		p = p->next;
	}
	strcat(buf, ")\n");

	stat = append_log(f, buf);

	free(buf);
	return(stat);
}

