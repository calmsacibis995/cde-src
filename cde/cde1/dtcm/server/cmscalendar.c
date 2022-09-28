/* $XConsortium: cmscalendar.c /main/cde1_maint/3 1995/10/31 10:27:56 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)cmscalendar.c	1.36 96/10/02 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <time.h>

#include <sys/statvfs.h>
#include <syslog.h>
#include <netdb.h>

#ifdef SunOS
#include <sys/systeminfo.h>
#endif
#include "cmscalendar.h"
#include "access.h"
#include "attr.h"
#include "cm.h"
#include "rtable4.h"
#include "tree.h"
#include "list.h"
#include "log.h"
#include "appt4.h"		/* Internal appointment data structure */
#include "cmsdata.h"
#include "cmsentry.h"
#include "callback.h"
#include "garbage.h"
#include "repeat.h"
#include "v4ops.h"
#include "v5ops.h"
#include "insert.h"
#include "updateattrs.h"
#include "reminder.h"
#include "misc.h"
#include "convert4-5.h"
#include "laccess.h"
#include "lutil.h"

extern	int	debug;
extern	char	*pgname;
extern  int     first_garbage_collection_time;

/* global variable shared with parser.y */
_DtCmsCalendar *currentCalendar;

/* static variables used within the file */
static	_DtCmsCalendar	calendar_list;

#ifdef MT
/* lock to serialize parsing and timing */
pthread_mutex_t parse_lock;
#endif

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/

static CSA_return_code _LoadCalendar(_DtCmsCalendar *cal);
static void _CleanUpCalendar(_DtCmsCalendar *cal);
static _DtCmsCalendar* _MakeEmptyCalendar(char *name);
static CSA_return_code _LockCalendar(_DtCmsCalendar *cal, _DtCmsCalendar *prev);
static _DtCmsCalendar* _FindPrevCal(char *name);
static CSA_return_code get_file_owner(char *log, char **owner);
static char* get_calname(char *target);
static CSA_return_code init_cal_attrs(_DtCmsCalendar *cal);
static CSA_return_code _GetCalendarSize(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _GetNumberEntries(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _GetAccessList(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _GetCalendarName(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _GetCalendarOwner(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _GetServerVersion(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _GetDataVersion(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _GetProductId(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _GetSupportedVersion(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _CopyCalendarAttr(_DtCmsCalendar *cal, int index,
					cms_attribute *attr);
static CSA_return_code _CheckAccessList(cms_access_entry *alist, char *owner);

static _DtCmGetAttrFunc _GetAttrFuncPtrs[] =
	{	NULL,
		_GetAccessList,
		_GetCalendarName,
		_GetCalendarOwner,
		_GetCalendarSize,
		_CopyCalendarAttr,
		_CopyCalendarAttr,
		_CopyCalendarAttr,
		_CopyCalendarAttr,
		_GetNumberEntries,
		_GetProductId,
		_CopyCalendarAttr,
		_GetSupportedVersion,
		_CopyCalendarAttr,
		_GetServerVersion,
		_GetDataVersion,
		_CopyCalendarAttr
	};

/*****************************************************************************
 * extern functions
 *****************************************************************************/

/* 
 * Takes a named "empty" calendar and the desired owner of the calendar and
 * initializes the calendar.  cal is set to NULL is there is an error.
 */
extern CSA_return_code
_DtCmsInitCalendar(_DtCmsCalendar *cal, char *owner)
{
        CSA_return_code   stat;

	cal->fversion = _DtCMS_VERSION4;

	if ((stat = init_cal_attrs(cal)) != CSA_SUCCESS) {
	        _DtCmsFreeCalendar(cal, TRUE);
		return(stat);
	}

	if ((stat = _DtCm_set_string_attrval(cal->calendar,
	    &cal->attrs[CSA_CAL_ATTR_CALENDAR_NAME_I].value, CSA_VALUE_STRING))
	    != CSA_SUCCESS) {
		_DtCmsFreeCalendar(cal, TRUE);
		return(stat);
	}

	if ((stat = _DtCm_set_string_attrval(owner,
	    &cal->attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].value,
	    CSA_VALUE_CALENDAR_USER)) != CSA_SUCCESS) {
		_DtCmsFreeCalendar(cal, TRUE);
		return(stat);
	}
	if ((cal->owner = strdup(cal->attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].\
	    value->item.string_value)) == NULL) {
		_DtCmsFreeCalendar(cal, TRUE);
		return(CSA_E_INSUFFICIENT_MEMORY);
	}

	if ((cal->types = (int *)calloc(1, sizeof(int) *
	    (_DtCm_entry_name_tbl->size + 1))) == NULL) {
		_DtCmsFreeCalendar(cal, TRUE);
		return(CSA_E_INSUFFICIENT_MEMORY);
	} else
		_DtCm_get_attribute_types(_DtCm_entry_name_tbl->size,
			cal->types);

	if (!(cal->tree = rb_create(_DtCmsGetEntryKey, _DtCmsCompareEntry))) {
		_DtCmsFreeCalendar(cal, TRUE);
		return(CSA_E_INSUFFICIENT_MEMORY);
	}

	if (!(cal->list = hc_create(_DtCmsGetEntryKey, _DtCmsCompareRptEntry)))
	{
		_DtCmsFreeCalendar(cal, TRUE);
		return(CSA_E_INSUFFICIENT_MEMORY);
	}

	cal->cal_tbl = _DtCm_cal_name_tbl;
	cal->entry_tbl = _DtCm_entry_name_tbl;
	cal->num_entry_attrs = _DtCm_entry_name_tbl->size;
	cal->getattrfuncs = _GetAttrFuncPtrs;

	return(CSA_SUCCESS);
}

/*
 * Takes a complete and valid calendar as an argument.
 */
extern void
_DtCmsUnlockCalendar(_DtCmsCalendar *cal)
{
#ifdef MT
  pthread_mutex_lock(&cal->synch_lock);
  cal->owned = 0;
  pthread_cond_signal(&cal->wake); /* No effect if no threads waiting. */
  pthread_mutex_unlock(&cal->synch_lock);
#endif /* MT */
}

/* 
 * Takes a calendar as an argument.  The calendar is assumed to be complete
 * if incomplete is set to FALSE.
 */
extern void
_DtCmsFreeCalendar(_DtCmsCalendar *cal, boolean_t incomplete)
{
	_DtCmsCalendar *prev;

	if (cal == NULL)
	  return;

	prev = _FindPrevCal(cal->calendar);
	  
	prev->next = cal->next;

#ifdef MT
	pthread_mutex_lock(&cal->synch_lock);
	pthread_mutex_unlock(&cal->list_lock);
	pthread_mutex_unlock(&prev->list_lock);
	
        /* If there are other threads waiting, mark and return. */
	if (cal->wait_count > 0) {
	  if (incomplete)
	    cal->incomplete = 1;
	  else
	    cal->deleted = 1;
	  cal->owned = 0;
	  pthread_cond_signal(&cal->wake);
	  pthread_mutex_unlock(&cal->synch_lock);
	}
	else                 
	  _CleanUpCalendar(cal);
#else /* MT */
	_CleanUpCalendar(cal);
#endif /* MT */
}

/*
 * Called when loading data from callog file.
 * Types of data to be stored in the tree and list structures
 * depends on the version of the callog file.
 */
extern CSA_return_code
_DtCmsSetFileVersion(_DtCmsCalendar *cal, int version)
{
	cal->fversion = version;

	if (version == 1) {
		if (!(cal->tree = rb_create(_DtCmsGetApptKey,
		    _DtCmsCompareAppt))) {
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		if (!(cal->list = hc_create(_DtCmsGetApptKey,
		    _DtCmsCompareRptAppt))) {
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
	} else {
		if (!(cal->tree = rb_create(_DtCmsGetEntryKey,
		    _DtCmsCompareEntry))) {
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		if (!(cal->list = hc_create(_DtCmsGetEntryKey,
		    _DtCmsCompareRptEntry))) {
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		if (init_cal_attrs(cal) != CSA_SUCCESS)
			return (CSA_E_INSUFFICIENT_MEMORY);
	
		if ((cal->types = (int *)calloc(1, sizeof(int) *
		    (_DtCm_entry_name_tbl->size + 1))) == NULL)
		        return (CSA_E_INSUFFICIENT_MEMORY);
		else
			_DtCm_get_attribute_types(_DtCm_entry_name_tbl->size,
				cal->types);

	}

	cal->cal_tbl = _DtCm_cal_name_tbl;
	cal->entry_tbl = _DtCm_entry_name_tbl;

	return (CSA_SUCCESS);
}

/* return value:
 * 0		- file loaded successfully
 * FILEERROR	- file does not exist
 * CERROR	- something goes wrong
 */
extern CSA_return_code
start_log(_DtCmsCalendar *cal, char *filename)
{
	FILE	*f;
	extern void setinput(FILE *);
	extern int yyyparse();
	extern int yyywrap(FILE *);
	struct stat   statbuf;

	if (cal == NULL || filename == NULL)
		return(CSA_E_FAILURE);

	if ((f = fopen(filename, "r")) == NULL) {
	  if (errno == ENOENT) 
	    return(CSA_E_CALENDAR_NOT_EXIST);
	  else
	    return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	/* Get the modification time and block size of the
	 * calendar. This information will be used later when
	 * writing out appointments back to the callog file.
	 */
	if (stat(filename, &statbuf) == -1) {

		/* This was already done in the fopen() call
		 * previous to the stat() call; however, this
		 * check is here just in case something happens
		 * in between the fopen() and the stat() call.
		 */
		if (errno == ENOENT)
			return(CSA_E_CALENDAR_NOT_EXIST);
		else
			return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}
 
        /* If the callog file exists but is empty, remove it and return */
        /* so that it can be recreated. */
 
        if (statbuf.st_size == 0) {
           if (debug) 
              fprintf(stderr, "%s: callog file is empty\n", pgname);
           unlink(filename);
           return(CSA_E_CALENDAR_NOT_EXIST);
        }

	cal->modtime = statbuf.st_mtime;
	cal->blksize = statbuf.st_blksize;
#ifdef UNDEF
	if (debug) {
		char cbuf[26];

		ctime_r(&(cal->modtime), cbuf);
		fprintf(stderr,
			"Initial log file info:\n\tmodtime: %s\n\tblksize: %d\n",
			cbuf, cal->blksize);
	}
#endif


#ifdef MT
	/* Serialize parser. */
	pthread_mutex_lock(&parse_lock);
	if (debug)
	  printf("parser locked\n");
#endif
	setinput(f);
	currentCalendar = cal;
	if (yyyparse() == -1) {
		fprintf(stderr, "%s: unable to parse %s\n", pgname, filename);
		yyywrap(f);
#ifdef MT
		pthread_mutex_unlock(&parse_lock);
		if (debug)
		  printf("parser unlocked due to error\n");	
#endif
		return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}
	yyywrap(f);
#ifdef MT
	pthread_mutex_unlock(&parse_lock);	
	if (debug)
	  printf("parser unlocked\n");
#endif

	return(CSA_SUCCESS);
}

/* 
 * Returns an error or cal will point to a valid calendar or NULL.  Note: cal
 * may point to NULL when the function returns CSA_SUCCESS.
 */
extern CSA_return_code
_DtCmsFindCalendarInList(char *target, _DtCmsCalendar **cal)
{
  char            *name;
  _DtCmsCalendar  *prev;
  CSA_return_code stat;

  if ((name = get_calname(target)) == NULL)
    return(CSA_E_INSUFFICIENT_MEMORY);

  prev = _FindPrevCal(name);
  
  if (prev->next == NULL) 
    goto NOT_FOUND;
  if (strcmp (name, prev->next->calendar) != 0)
    goto NOT_FOUND;

  *cal = prev->next;

  if ((stat = _LockCalendar(prev->next, prev)) != CSA_SUCCESS) {
    *cal = NULL;
    free(name);
    return(stat);
  }

  free(name);
  return(CSA_SUCCESS);

NOT_FOUND:
#ifdef MT
  if (prev->next != NULL)
    pthread_mutex_unlock(&prev->next->list_lock);
  pthread_mutex_unlock(&prev->list_lock);
#endif
  *cal = NULL;
  free(name);
  return(CSA_SUCCESS);
}

/* 
 * Adds an empty calendar to the calendar list if the calendar does not already
 * exist.
 */
extern CSA_return_code
_DtCmsAddCalendarToList(char *target, _DtCmsCalendar **cal)
{
  char            *name;
  _DtCmsCalendar  *prev;
  CSA_return_code stat;

  if ((name = get_calname(target)) == NULL)
    return(CSA_E_INSUFFICIENT_MEMORY);

  prev = _FindPrevCal(name);

  if ((prev->next != NULL) && (strcmp (name, prev->next->calendar) == 0) &&
	 *cal != (_DtCmsCalendar*)-1) {
#ifdef MT
    pthread_mutex_unlock(&prev->next->list_lock);
    pthread_mutex_unlock(&prev->list_lock);
#endif
    free(name);
    return(CSA_E_CALENDAR_EXISTS);
  }
#ifdef MT
  if ((*cal = _MakeEmptyCalendar(name)) == NULL) {
    pthread_mutex_unlock(&prev->list_lock);
    if (prev->next != NULL)
      pthread_mutex_unlock(&prev->next->list_lock);
    free(name);
    return(CSA_E_INSUFFICIENT_MEMORY);
  }
  pthread_mutex_lock(&(*cal)->list_lock);
  (*cal)->next = prev->next;
  prev->next = *cal;
  if ((*cal)->next != NULL)
    pthread_mutex_unlock(&(*cal)->next->list_lock);
  if ((stat = _LockCalendar(*cal, prev)) != CSA_SUCCESS) {
    free(name);
    return(stat);
  }
#else /* MT */
  if ((*cal = _MakeEmptyCalendar(name)) == NULL) {
    free(name);
    return(CSA_E_INSUFFICIENT_MEMORY);
  }
  (*cal)->next = prev->next;
  prev->next = *cal;
#endif /* MT */
  free(name);
  if ((stat = _LoadCalendar(*cal)) != CSA_E_CALENDAR_NOT_EXIST) {
    if (stat != CSA_SUCCESS)
      _DtCmsFreeCalendar(*cal, TRUE);
    else
      _DtCmsUnlockCalendar(*cal);
    return(CSA_E_CALENDAR_EXISTS);
  }
  else
    return(CSA_SUCCESS);
}

/* 
 * The cal handle will point to a complete calendar or, in case of error,  a 
 * deleted calendar.
 */
extern CSA_return_code
_DtCmsGetCalendarByName(char *target, _DtCmsCalendar **cal)
{
  char            *name;
  _DtCmsCalendar  *prev;
  CSA_return_code stat;

  if ((name = get_calname(target)) == NULL)
    return(CSA_E_INSUFFICIENT_MEMORY);

  prev = _FindPrevCal(name);
  
  if ((prev->next != NULL) && (strcmp (name, prev->next->calendar) == 0)) {
    *cal = prev->next;
    if ((stat = _LockCalendar(prev->next, prev)) != CSA_SUCCESS) {
      *cal = NULL;
      free(name);
      return(stat);
    }
    else {
      free(name);
      return(CSA_SUCCESS);
    }
  }
#ifdef MT
  if ((*cal = _MakeEmptyCalendar(name)) == NULL) {
    pthread_mutex_unlock(&prev->list_lock);
    if (prev->next != NULL)
      pthread_mutex_unlock(&prev->next->list_lock);
    free(name);
    return(CSA_E_INSUFFICIENT_MEMORY);
  }
  pthread_mutex_lock(&(*cal)->list_lock);
  (*cal)->next = prev->next;
  prev->next = *cal;
  if ((*cal)->next != NULL)
    pthread_mutex_unlock(&(*cal)->next->list_lock);
  if ((stat = _LockCalendar(*cal, prev)) != CSA_SUCCESS) {
    free(name);
    return(stat);
  }
#else /* MT */
  if ((*cal = _MakeEmptyCalendar(name)) == NULL) {
    free(name);
    return(CSA_E_INSUFFICIENT_MEMORY);
  }
  (*cal)->next = prev->next;
  prev->next = *cal;
#endif /* MT */
  free(name);
  if ((stat = _LoadCalendar(*cal)) != CSA_SUCCESS) {
    if (stat == CSA_E_CALENDAR_NOT_EXIST)
      _DtCmsFreeCalendar(*cal, FALSE);
    else
      _DtCmsFreeCalendar(*cal, TRUE);
  }
  return(stat);
}

/* 
 * The cal handle will point to a complete calendar or, in case of error,  a 
 * deleted calendar.
 */
extern CSA_return_code
_DtCmsLoadCalendar(_DtCmsCalendar *cal)
{
  CSA_return_code stat;

  if ((stat = _LoadCalendar(cal)) != CSA_SUCCESS) {
    if (stat == CSA_E_CALENDAR_NOT_EXIST)
      _DtCmsFreeCalendar(cal, FALSE);
    else
      _DtCmsFreeCalendar(cal, TRUE);
  }
  return(stat);
}

/*
 * For parser only.
 */
extern CSA_return_code
_DtCmsInsertAppt_for_parser(_DtCmsCalendar *cal, Appt_4 *appt)
{
	return (_DtCmsInsertAppt(cal, appt));
}

/*
 * For parser only.
 */
extern CSA_return_code
_DtCmsInsertEntry4Parser(_DtCmsCalendar *cal, cms_entry *entry)
{
	cms_entry	*eptr;
	CSA_return_code stat;


	/* If the entry in the callog file has no attributes, then
	 * assume that the callog file is corrupt!  Each appointment
	 * entry should have attributes.
	 */
	if (entry->num_attrs == 0) {
		return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}


	if ((stat = _DtCmsMakeHashedEntry(cal, entry->num_attrs,
	    entry->attrs, &eptr)) != CSA_SUCCESS)
		return (stat);

	if (eptr) {
		eptr->key = entry->key;

		stat = _DtCmsInsertEntry(cal, eptr);

		_DtCm_free_cms_entry(eptr);
	}

	return(stat);
}

extern void
_DtCmsGenerateKey(_DtCmsCalendar *cal, long *key)
{
	if (*key == 0) {
		*key = ++(cal->lastkey);
	} else if (*key > cal->lastkey)
		cal->lastkey = *key;
}

extern CSA_return_code
_DtCmsEnumerateUp(_DtCmsCalendar *cal, _DtCmsEnumerateProc doit)
{
	if (cal == NULL || doit == NULL)
		return (CSA_E_INVALID_PARAMETER);
	if (rb_enumerate_up (APPT_TREE(cal), doit) != rb_ok ||
	    hc_enumerate_up (REPT_LIST(cal), doit) != rb_ok)
		return (CSA_E_FAILURE);
	else
		return (CSA_SUCCESS);
}

extern void
_DtCmsEnumerateDown(_DtCmsCalendar *cal, _DtCmsEnumerateProc doit)
{
	if (cal == NULL || doit == NULL)
		return;
	rb_enumerate_down (APPT_TREE(cal), doit);
	hc_enumerate_down (REPT_LIST(cal), doit);
}

/*
 * For parser only.
 */
extern void
_DtCmsSetAccess4Parser(
	_DtCmsCalendar *cal,
	Access_Entry_4 *list,
	int type)
{
	if (type == access_read_4) {
		_DtCm_free_access_list4(cal->r_access);
		cal->r_access = list;
	} else if (type == access_write_4) {
		_DtCm_free_access_list4(cal->w_access);
		cal->w_access = list;
	} else if (type == access_delete_4) {
		_DtCm_free_access_list4(cal->d_access);
		cal->d_access = list;
	} else if (type == access_exec_4) {
		_DtCm_free_access_list4(cal->x_access);
		cal->x_access = list;
	}
}

extern void
_DtCmsSetCalendarAttrs4Parser(
	_DtCmsCalendar *cal,
	int len,
	cms_attribute *attrs)
{
	cms_attribute_value *val;

	/* we don't want to free the value part of the access list
	 * attribute even it may be an empty list
	 */
	val = cal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value;
	cal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value = NULL;
	_DtCm_free_cms_attribute_values(cal->num_attrs, cal->attrs);
	cal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value = val;

	(void)_DtCmsUpdateAttributes(len, attrs, &cal->num_attrs, &cal->attrs,
		&cal->cal_tbl, _B_TRUE, NULL);
}

extern CSA_return_code
_DtCmsRbToCsaStat(Rb_Status rb_stat)
{
	switch (rb_stat) {
	case rb_ok:
		return (CSA_SUCCESS);
	case rb_duplicate:
	case rb_badtable:
	case rb_notable:
	case rb_failed:
	case rb_other:
		return (CSA_E_FAILURE);
	}
}

#ifdef MT
extern void *
garbage_collect(void *arg)
#else /* MT */
extern void
garbage_collect()
#endif /* MT */
{
  unsigned remain;
  _DtCmsCalendar *clist, *prev;

  if (debug)
    printf("Garbage Collect\n");

#ifdef MT
  sleep(first_garbage_collection_time);

  for (;;) {
BEGINNING:
    prev = &calendar_list;
    pthread_mutex_lock(&prev->list_lock);
    while ((clist = prev->next) != NULL) {
      pthread_mutex_lock(&clist->list_lock);
      if (_LockCalendar(clist, prev) == CSA_SUCCESS) {
	/* deregister stale client */
	clist->rlist = _DtCmsCheckRegistrationList(clist->rlist);
	if (clist->remq)
	  _DtCmsUpdateReminders(clist->remq);
	if (clist->modified == _B_TRUE) {
	  if (debug)
	    fprintf(stderr, "rpc.csmd: %s%s\n", "do garbage collection on ",
		  clist->calendar);
	  _DtCmsCollectOne(clist);
	  clist->modified = _B_FALSE;
	}
	pthread_mutex_lock(&clist->list_lock);
	_DtCmsUnlockCalendar(clist);
      }
      else
	goto BEGINNING; /* Need to start over from the beginning. */
      prev = clist;
    }
    pthread_mutex_unlock(&prev->list_lock);
    remain = 3600 * 24;
    if (debug)
      printf("Time: %d, Sleeping Until: %d\n", time(0), time(0)+remain);
    sleep((int)remain);
  }

#else /* MT */
  prev = &calendar_list;
  while ((clist = prev->next) != NULL) {
    /* deregister stale client */
    clist->rlist = _DtCmsCheckRegistrationList(clist->rlist);
    if (clist->remq)
      _DtCmsUpdateReminders(clist->remq);
    if (clist->modified == _B_TRUE) {
      if (debug)
	fprintf(stderr, "rpc.csmd: %s%s\n", "do garbage collection on ",
		clist->calendar);
      _DtCmsCollectOne(clist);
      clist->modified = _B_FALSE;
    }
    prev = clist;
  }
  if ((remain = (unsigned) alarm((unsigned) 0)) == 0) {
    remain = 3600L * 24L;
    if (debug)
      printf("Resetting Alarm to %d\n", time(0)+remain);
  }
  alarm ((unsigned) remain);
#endif /* MT */
}

extern void
debug_switch()
{
  _DtCmsCalendar *clist, *prev;

  /* Changing the debug global is not MT-safe */
  debug = !debug;
  fprintf (stderr, "Debug Mode is %s\n", debug ? "ON" : "OFF");

#ifndef MT
  if (debug) {
    prev = &calendar_list;
    while ((clist = prev->next) != NULL) {
      _DtCmsPrintReminderListV4(clist->rm_queue);
      _DtCmsShowAccessList(clist->alist);
      prev = clist;
    }
  }
#endif /* MT */
}

extern CSA_return_code
_DtCmsGetCalAttrsByName(
	_DtCmsCalendar	*cal,
	CSA_uint32	num_names,
	cms_attr_name	*names,
	CSA_uint32	*num_attrs_r,
	cms_attribute	**attrs_r)
{
	CSA_return_code	stat = CSA_SUCCESS;
	int		i, j, index;
	cms_attribute	*attrs;

	if ((attrs = calloc(1, sizeof(cms_attribute)*num_names)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* get attributes */
	for (i = 0, j = 0; i < num_names; i++) {
		if (names[i].name == NULL)
			continue;

		if (names[i].num <= 0)
			index = names[i].num = _DtCm_get_index_from_table(
					cal->cal_tbl, names[i].name);

		if (index > 0) {
			if (index <= _DtCM_DEFINED_CAL_ATTR_SIZE) {
				if (_CSA_cal_attr_info[index].fst_vers
				    <= cal->fversion
				    || index == CSA_CAL_ATTR_CALENDAR_SIZE_I) {
					stat = cal->getattrfuncs[index](cal,
						index, &attrs[j]);
				}
			} else if (cal->attrs && cal->attrs[index].value) {
				stat = _DtCm_copy_cms_attribute(&attrs[j],
					&cal->attrs[index], _B_TRUE);
			}

			if (stat != CSA_SUCCESS) {
				_DtCm_free_cms_attributes(j, attrs);
				free(attrs);
				return (stat);
			} else if (attrs[j].name.name)
				j++;
		}
	}

	if (j > 0) {
		*num_attrs_r = j;
		*attrs_r = attrs;
	} else {
		*num_attrs_r = 0;
		*attrs_r = NULL;
		free(attrs);
	}

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmsGetAllCalAttrs(
	_DtCmsCalendar	*cal,
	CSA_uint32	*num_attrs_r,
	cms_attribute	**attrs_r,
	boolean_t	returnall)
{
	CSA_return_code	stat = CSA_SUCCESS;
	cms_attribute	*attrs, *cattrs = cal->attrs;
	int		i, j, num_cal_attrs = cal->num_attrs;

	num_cal_attrs = cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION ?
			cal->num_attrs : _DtCM_OLD_CAL_ATTR_SIZE;

	if ((attrs = calloc(1, sizeof(cms_attribute)*num_cal_attrs)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		/* first element is not used */
		for (i = 1, j = 0; i <= num_cal_attrs; i++) {
			if (i <= _DtCM_DEFINED_CAL_ATTR_SIZE) {
				if ((stat = cal->getattrfuncs[i](cal, i,
				    &attrs[j])) != CSA_SUCCESS) {
					_DtCm_free_cms_attributes(j, attrs);
					free(attrs);
					return (stat);
				} else if (attrs[j].name.name)
					j++;
			} else if (cattrs[i].value) {
				if (returnall) {
					stat = _DtCm_copy_cms_attribute(
						&attrs[j], &cal->attrs[i],
						_B_TRUE);
				} else {
					/* just return the attribute name */
					if (attrs[j].name.name = strdup(
					    cattrs[i].name.name)) {
						attrs[j].name.num =
							cattrs[i].name.num;
					} else
						stat =CSA_E_INSUFFICIENT_MEMORY;
				}

				if (stat == CSA_SUCCESS)
					j++;
				else {
					_DtCm_free_cms_attributes(j, attrs);
					free(attrs);
					return (stat);
				}
			}
		}
	} else {
		for (i = 1, j = 0; i <= _DtCM_DEFINED_CAL_ATTR_SIZE-1; i++) {
			if (_CSA_cal_attr_info[i].fst_vers <= cal->fversion ||
			    i == CSA_CAL_ATTR_CALENDAR_SIZE_I)
			{
				if ((stat = cal->getattrfuncs[i](cal, i,
				    &attrs[j])) != CSA_SUCCESS) {
					_DtCm_free_cms_attributes(j, attrs);
					free(attrs);
					return (stat);
				} else if (attrs[j].name.name)
					j++;
			}
		}
	}

	if (j > 0) {
		*num_attrs_r = j;
		*attrs_r = attrs;
	} else {
		*num_attrs_r = 0;
		*attrs_r = NULL;
		free(attrs);
	}

	return (CSA_SUCCESS);
}

extern CSA_return_code
_DtCmsGetCalAttrNames(
	_DtCmsCalendar	*cal,
	CSA_uint32	*num_names_r,
	cms_attr_name	**names_r)
{
	cms_attribute	*attrs = cal->attrs;
	cms_attr_name	*names;
	CSA_uint32	i, j, num_attrs = cal->num_attrs;

	if ((names = calloc(1, sizeof(cms_attr_name)*num_attrs)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);

	/* first element is not used */
	for (i = 1, j = 0; i <= num_attrs; i++) {
		if (attrs[i].value || (i == CSA_CAL_ATTR_CALENDAR_SIZE_I ||
		    i == CSA_CAL_ATTR_NUMBER_ENTRIES_I ||
		    i == CSA_X_DT_CAL_ATTR_SERVER_VERSION_I ||
		    i == CSA_X_DT_CAL_ATTR_DATA_VERSION_I ||
		    i == CSA_CAL_ATTR_ACCESS_LIST_I))
		{
			if (names[j].name = strdup(attrs[i].name.name)) {
				names[j].num = attrs[i].name.num;
				j++;
			} else {
				_DtCmsFreeCmsAttrNames(j, names);
				return (CSA_E_INSUFFICIENT_MEMORY);
			}
		}
	}

	if (j > 0) {
		*num_names_r = j;
		*names_r = names;
	} else {
		*num_names_r = 0;
		*names_r = NULL;
		free(names);
	}

	return (CSA_SUCCESS);
}

extern void
_DtCmsFreeCmsAttrNames(CSA_uint32 num, cms_attr_name *names)
{
	CSA_uint32	i;

	for (i = 0; i < num; i++)
		if (names[i].name) free(names[i].name);

	free(names);
}

/*
 * This routine assumes that the attribute names are hashed
 * and the associated hash number can be used without checking.
 * Will hash the name only if the hash number is <= 0.
 */
extern CSA_return_code
_DtCmsUpdateCalAttributesAndLog(
	_DtCmsCalendar	*cal,
	CSA_uint32	numsrc,
	cms_attribute	*srcattrs,
	CSA_uint32	access)
{
	CSA_return_code stat = CSA_SUCCESS;
	CSA_uint32	i, oldnum = 0;
	cms_attribute	*oldattrs = NULL;
	char		*log;

	/* we didn't use _DtCmUpdateAttributes because we need
	 * to check access rights here
	 */

	/* copy original attributes for rollback if update fails */
	if (cal->attrs && (stat = _DtCm_copy_cms_attributes(cal->num_attrs,
	    cal->attrs, &oldnum, &oldattrs)) != CSA_SUCCESS)
		return (stat);

	for (i = 0; i < numsrc && stat == CSA_SUCCESS; i++) {
		if (srcattrs[i].name.name == NULL)
			continue;

		if (srcattrs[i].name.num <= 0)
			srcattrs[i].name.num = _DtCm_get_index_from_table(
						cal->cal_tbl,
						srcattrs[i].name.name);

		if (srcattrs[i].name.num < 0) {
			/* new attribute */

			if (!_DTCMS_HAS_INSERT_CALENDAR_ATTR_ACCESS(access))
				stat = CSA_E_NO_AUTHORITY;
			else {
				if ((stat = _DtCmExtendNameTable(
				    srcattrs[i].name.name, 0, 0,
				    _DtCm_cal_name_tbl,
				    _DtCM_DEFINED_CAL_ATTR_SIZE,
				    CSA_CALENDAR_ATTRIBUTE_NAMES,
				    &cal->cal_tbl, NULL)) == CSA_SUCCESS) {
					srcattrs[i].name.num =
						cal->cal_tbl->size;
					stat = _DtCmGrowAttrArray(
						&cal->num_attrs, &cal->attrs,
						&srcattrs[i]);
				}
			}

		} else if (!_DTCMS_HAS_CHANGE_CALENDAR_ATTR_ACCESS(access)) {

			stat = CSA_E_NO_AUTHORITY;
		} else {
			cms_attribute_value val;

			if (srcattrs[i].name.num == CSA_CAL_ATTR_ACCESS_LIST_I)
			{
				if (srcattrs[i].value == NULL) {
					val.type = CSA_VALUE_ACCESS_LIST;
					val.item.access_list_value = NULL;
					srcattrs[i].value = &val;
				} else {
					stat = _CheckAccessList(srcattrs[i].\
						value->item.access_list_value,
						cal->owner);
					if (stat != CSA_SUCCESS)
						break;
				}
			}

			stat = _DtCmUpdateAttribute(&srcattrs[i],
				&cal->attrs[srcattrs[i].name.num]);
		}
	}

	if (stat == CSA_SUCCESS) {
		log = _DtCmsGetLogFN(cal->calendar);

		/* dump file */
		stat = _DtCmsAppendCalAttrsByFN(log, cal->num_attrs,
			cal->attrs);
		free(log);
	}

	if (stat == CSA_SUCCESS) {
		cal->modified = _B_TRUE;

		_DtCm_free_cms_attributes(oldnum + 1, oldattrs);
		free(oldattrs);
	} else {
		_DtCm_free_cms_attributes(cal->num_attrs+1, cal->attrs);
		free(cal->attrs);
		cal->num_attrs = oldnum;
		cal->attrs = oldattrs;
	}

	return (stat);
}

extern CSA_return_code
_DtCmsV5TransactLog(_DtCmsCalendar *cal, cms_entry *e, _DtCmsLogOps op)
{
	CSA_return_code	stat;
	char		*log;

	if ((log = _DtCmsGetLogFN(cal->calendar)) == NULL)
		 return (CSA_E_INSUFFICIENT_MEMORY);

	if (cal->hashed == _B_FALSE ||
	    cal->num_entry_attrs < cal->entry_tbl->size) {
		if ((stat = _DtCmsAppendHTableByFN(log, cal->entry_tbl->size,
		    cal->entry_tbl->names, cal->types)) != CSA_SUCCESS) {
			free(log);
			return (stat);
		} else {
			cal->hashed = _B_TRUE;
			cal->num_entry_attrs = cal->entry_tbl->size;
		}
	}
	stat = _DtCmsAppendEntryByFN(log, e, op);
	free(log);
	return(stat);
}

/*
 * If owner is not root and we find the owner's name in the list,
 * we need to check the access right at each operation.
 */
extern void
_DtCmsMarkCheckOwner(_DtCmsCalendar *cal)
{
	char		*ownername, *username;

	/* reset it to _B_FALSE first */
	cal->checkowner = _B_FALSE;
	ownername = _DtCmGetPrefix(cal->owner, '@');

	if (strcmp(ownername, _DTCMS_SUPER_USER) == 0)
		goto done;

	if (cal->fversion == _DtCMS_VERSION1) {
		Access_Entry_4	*alist;

		for (alist = cal->alist;
		     alist != NULL && cal->checkowner == _B_FALSE;
		     alist = alist->next)
		{
			username = _DtCmGetPrefix(alist->who, '@');
			if (strcmp(ownername, username) == 0) {
				cal->checkowner = _B_TRUE;
			}
			free(username);
		}
	} else {
		cms_access_entry	*alist;

		alist = cal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I].value->item.\
				access_list_value;

		for ( ; alist != NULL && cal->checkowner == _B_FALSE;
		     alist = alist->next)
		{
			username = _DtCmGetPrefix(alist->user, '@');
			if (strcmp(ownername, username) == 0) {
				cal->checkowner = _B_TRUE;
			}
			free(username);
		}
	}

done:
	free(ownername);
}

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

/*
 * Load calendar data into memory from callog file.
 * Assumes that the calendar is locked, incomplete, and in the calendar list.
 * If there is an error, the calendar may be complete or incomplete.
 */
static CSA_return_code
_LoadCalendar(_DtCmsCalendar *cal)
{
	CSA_return_code	stat;
	char		*log = NULL;

	char directory[64], *path;
	struct statvfs file_info; 
	struct statvfs *buf;

	long   count, retcode;
	char * localhost_ptr;
	char * parsed_hostname;
	char * parsed_username; 
	int    end_pos, at_sign_pos, i;

	char * input_calendar;
	char * calendar_buf;
	char * ptr;
        char * msg_ptr;

	input_calendar  = (char *)malloc(MAXHOSTNAMELEN + 1);
	parsed_hostname = (char *)malloc(MAXHOSTNAMELEN + 1); 
	calendar_buf    = (char *)malloc(MAXHOSTNAMELEN + 1);
	parsed_username = (char *)malloc(MAXHOSTNAMELEN + 1); 
        localhost_ptr = 0; 
 
        buf = &file_info;
        path =  directory;
        sprintf(path,"%s","/var/spool/calendar/");  /* ufs ? */ 
        sprintf(input_calendar,"%s", cal->calendar);
	sprintf(parsed_username,"callog.%s",input_calendar); 

        /* *******************************************************
        **          NFS OR UFS ???
        */
        strcat(path, parsed_username);
        retcode =  statvfs(path, buf);
     
        if (!strcmp(buf->f_basetype,"nfs")) 
        {
	   localhost_ptr = (char *)malloc(MAXHOSTNAMELEN + 1); 
	   count = MAXHOSTNAMELEN + 1;
           retcode = sysinfo(SI_HOSTNAME, localhost_ptr, count);
           sprintf(input_calendar,"%s@%s",cal->calendar,localhost_ptr);

           msg_ptr = (char *)malloc(256);
           sprintf(msg_ptr,
           "rpc.cmsd : NFS mounted callog file Not Supported - %s\n",input_calendar);
           syslog(LOG_ALERT, msg_ptr);
           free(msg_ptr);
                         
           if (debug) 
              fprintf(stderr," NFS mounted callog file Not Supported: %s\n",input_calendar); 
           stat = CSA_X_DT_E_NFS_MOUNTED_CALLOG;
        }
     
        /*  free mallocs  */
        if(localhost_ptr) free(localhost_ptr);
        free(input_calendar);
        free(parsed_hostname);
        free(parsed_username);
        free(calendar_buf); 

        if(stat == CSA_X_DT_E_NFS_MOUNTED_CALLOG) goto ERROR;


	if ((cal == NULL) || (cal->calendar == NULL)) {
	  if (debug)
	    printf("_LoadCalendar: called without empty calendar\n");
	  stat = CSA_E_FAILURE;
	  goto ERROR;
	}


 	if ((log = _DtCmsGetLogFN(cal->calendar)) == NULL) { 
	  stat = CSA_E_INSUFFICIENT_MEMORY;
	  goto ERROR;
 	}    


	/* Insrtumentation for timing. */
	if (debug)
	  printf("before loading: %d\n", time(0));
	/* load data from file */
	if ((stat = start_log(cal, log)) != CSA_SUCCESS)
		goto ERROR;
	if (debug)
	  printf("after loading: %d\n", time(0));

	/*
	 * get file owner after the file is loaded since file
	 * ownership might be fixed in start_log()
	 */
	if (cal->fversion == _DtCMS_VERSION1) {

		if ((stat = get_file_owner(cal->calendar, &cal->owner))
		    != CSA_SUCCESS)
			goto ERROR;

		cal->alist = _DtCmsCalendarAccessList(cal);

	} else {
		if (cal->attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].value)
			cal->owner = strdup(cal->\
					attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I].\
					value->item.calendar_user_value);
		else {
			stat = CSA_X_DT_E_BACKING_STORE_PROBLEM;
	    		fprintf(stderr, "%s: %s\n", pgname,
				"calendar owner attribute is missing from callog file");
			goto ERROR;
		}

		if (cal->attrs[CSA_CAL_ATTR_PRODUCT_IDENTIFIER_I].value == NULL)
		{
			if ((stat = _DtCm_set_string_attrval(
			    _DtCM_PRODUCT_IDENTIFIER, &cal->\
			    attrs[CSA_CAL_ATTR_PRODUCT_IDENTIFIER_I].value,
			    CSA_VALUE_STRING)) != CSA_SUCCESS) {
				goto ERROR;
			}
		}

		if (cal->attrs[CSA_CAL_ATTR_VERSION_I].value == NULL) {
			if ((stat = _DtCm_set_string_attrval(
			    _DtCM_SPEC_VERSION_SUPPORTED,
			    &cal->attrs[CSA_CAL_ATTR_VERSION_I].value,
			    CSA_VALUE_STRING)) != CSA_SUCCESS) {
				goto ERROR;
			}
		}

		cal->num_entry_attrs = cal->entry_tbl->size;
	}

	cal->getattrfuncs = _GetAttrFuncPtrs;

	/* mark checkowner flag */
	_DtCmsMarkCheckOwner(cal);

	free(log);

	if (debug)
	  printf("_LoadCalendar: Loaded %s whose owner is %s.\n", cal->calendar, cal->owner);

	return (CSA_SUCCESS);
ERROR:
	if (log)
	  free(log);

	return (stat);
}

static void
_CleanUpCalendar(_DtCmsCalendar *cal)
{
	if (cal->calendar)
		free(cal->calendar);

	if (cal->owner)
		free(cal->owner);

	if (cal->fversion == 1) {
		_DtCm_free_access_list4(cal->r_access);
		_DtCm_free_access_list4(cal->w_access);
		_DtCm_free_access_list4(cal->d_access);
		_DtCm_free_access_list4(cal->x_access);
		_DtCm_free_access_list4(cal->alist);
	} else {
		if (cal->cal_tbl != _DtCm_cal_name_tbl)
			_DtCm_free_name_table(cal->cal_tbl);

		if (cal->entry_tbl != _DtCm_entry_name_tbl)
			_DtCm_free_name_table(cal->entry_tbl);

		if (cal->types) free(cal->types);

		if (cal->num_attrs) {
			_DtCm_free_cms_attributes(cal->num_attrs + 1,
				cal->attrs);
			free(cal->attrs);
		}
	}

	if (cal->tree)
		rb_destroy(cal->tree, NULL);

	if (cal->list)
		hc_destroy(cal->list, NULL);

	free(cal);
}

/* 
 * Takes the name of the new calendar and returns an empty calendar or NULL
 * if there is insufficient memory.
 */
static _DtCmsCalendar *
_MakeEmptyCalendar(char *name)
{
	_DtCmsCalendar	*cal;

	if ((cal = (_DtCmsCalendar *)calloc(1, sizeof(_DtCmsCalendar)))
	    == NULL)
	  return (NULL);

	if ((cal->calendar = get_calname(name)) == NULL) {
	  free(cal);
	  return (NULL);
	}

	return (cal);
}

/* 
 * Takes pointers to the calendar to be locked and the calendar preceeding it
 * in the calendar list.  Both calendars must have their list_locks locked.
 * May return CSA_E_CALENDAR_NOT_EXIST if the calendar has been deleted.
 */
static CSA_return_code
_LockCalendar(_DtCmsCalendar *cal, _DtCmsCalendar *prev)
{
  CSA_return_code stat;

#ifdef MT
  pthread_mutex_lock(&cal->synch_lock);
  pthread_mutex_unlock(&cal->list_lock);
  pthread_mutex_unlock(&prev->list_lock);

  /* 
   * Must re-evaluate cal->owned because pthread_cond_signal "unblocks at
   * least one thread" instead of only one thread.
   */
  cal->wait_count++;
  while (cal->owned)
    pthread_cond_wait(&cal->wake, &cal->synch_lock);
  cal->wait_count--;

  if ((cal->deleted == 0) && (cal->incomplete == 0)) {
    cal->owned = 1;
    pthread_mutex_unlock(&cal->synch_lock);
    return(CSA_SUCCESS);
  }
  else {
    if (cal->deleted)
      stat = CSA_E_CALENDAR_NOT_EXIST;
    else
      stat = CSA_E_INSUFFICIENT_MEMORY;
    if (cal->wait_count == 0) {
      cal->owned = 1;
      pthread_mutex_unlock(&cal->synch_lock);
      _CleanUpCalendar(cal);
    }
    else {
      pthread_cond_signal(&cal->wake);
      pthread_mutex_unlock(&cal->synch_lock);
    }
    return(stat);
  }
#else /* MT */
  return(CSA_SUCCESS);
#endif /* MT */
}

static _DtCmsCalendar*
_FindPrevCal(char *name)
{
  _DtCmsCalendar *clist, *prev;

#ifdef MT
  prev = &calendar_list;
  pthread_mutex_lock(&prev->list_lock);
  while ((clist = prev->next) != NULL) {
    pthread_mutex_lock(&clist->list_lock);
    if (strcmp(name, clist->calendar) <= 0)
      return prev;
    pthread_mutex_unlock(&prev->list_lock);
    prev = clist;
  }
  return (prev);
#else /* MT */
  prev = &calendar_list;
  while ((clist = prev->next) != NULL) {
    if (strcmp(name, clist->calendar) <= 0)
      return prev;
    prev = clist;
  }
  return (prev);
#endif /* MT */
}

/*
 * return user name of file owner
 * If the calendar name is the same as a user account name, the
 * calendar name is return; otherwise, the owner name is derived from
 * the owner of the calendar file
 */
static CSA_return_code
get_file_owner(char *calname, char **owner)
{
	struct stat info;
	char *log, *name;
	int res;
	struct passwd *pw;
#ifdef SunOS
	struct passwd pwstruct;
	char buf[BUFSIZ];
#endif

	if (calname == NULL || owner == NULL)
		return (CSA_E_FAILURE);

	*owner = NULL;

	if (_DtCmIsUserName(calname) == _B_TRUE) {
		name = calname;
	} else {
		if ((log = _DtCmsGetLogFN(calname)) == NULL)
			return (CSA_E_INSUFFICIENT_MEMORY);

		res = stat(log, &info);
		free(log);

		if (res == 0) {
#ifdef SunOS
		  	int status;
			if (status=getpwuid_r(info.st_uid,
					      &pwstruct, buf,
					      BUFSIZ-1, &pw) == 0)
#else
			if (pw = getpwuid(info.st_uid))
#endif
				name = pw->pw_name;
			else
				return (CSA_E_FAILURE);
		} else
			return(CSA_X_DT_E_BACKING_STORE_PROBLEM);
	}

	if (((*owner) = (char *)strdup(name)) == NULL)
		return (CSA_E_INSUFFICIENT_MEMORY);
	else
		return (CSA_SUCCESS);
}

static char *
get_calname(char *target)
{
	char *ptr, *name, *tmp;

	if (target == NULL)
		return (NULL);

	tmp = strdup(target);
	ptr = strchr(tmp, '@');
	if (ptr == NULL) {
	        free(tmp);
		return (strdup(target));
	} else {
		*ptr = NULL;
		name = strdup(tmp);
		free(tmp);
		return (name);
	}
}

static CSA_return_code
init_cal_attrs(_DtCmsCalendar *cal)
{
	int i;

	/* initialize the calendar with predefined attribute names */
	if ((cal->attrs = (cms_attribute *)calloc(1,
	    sizeof(cms_attribute)*(_DtCm_cal_name_tbl->size + 1))) == NULL) {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}

	for (i = 1; i <= _DtCm_cal_name_tbl->size; i++) {
		if ((cal->attrs[i].name.name =
		    strdup(_DtCm_cal_name_tbl->names[i])) == NULL) {
			_DtCm_free_cms_attributes(i, cal->attrs);
			free(cal->attrs);
			return (CSA_E_INSUFFICIENT_MEMORY);
		}

		cal->attrs[i].name.num = i;
	}
	cal->num_attrs = _DtCm_cal_name_tbl->size;

	return (CSA_SUCCESS);
}

static CSA_return_code
_CopyCalendarAttr(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	if (cal->attrs && cal->attrs[index].value)
		return (_DtCm_copy_cms_attribute(attr,
				&cal->attrs[index], _B_TRUE));
	else
		return (CSA_SUCCESS);
}

static CSA_return_code
_GetCalendarSize(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	CSA_return_code	stat;
	int		size;

	if ((stat = _DtCmsGetFileSize(cal->calendar, &size)) == CSA_SUCCESS) {
		attr->name.num = CSA_CAL_ATTR_CALENDAR_SIZE_I;
		if (attr->name.name = strdup(CSA_CAL_ATTR_CALENDAR_SIZE))
			return (_DtCm_set_uint32_attrval((CSA_uint32)size,
				&attr->value));
		else
			return (CSA_E_INSUFFICIENT_MEMORY);
	}
	return (stat);
}

static CSA_return_code
_GetNumberEntries(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	CSA_uint32	size;

	size = rb_size(cal->tree) + hc_size(cal->list);

	attr->name.num = CSA_CAL_ATTR_NUMBER_ENTRIES_I;
	if (attr->name.name = strdup(CSA_CAL_ATTR_NUMBER_ENTRIES))
		return (_DtCm_set_uint32_attrval(size, &attr->value));
	else
		return (CSA_E_INSUFFICIENT_MEMORY);
}

static CSA_return_code
_GetAccessList(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		return (_DtCm_copy_cms_attribute(attr,
			&cal->attrs[CSA_CAL_ATTR_ACCESS_LIST_I], _B_TRUE));
	} else {
		CSA_return_code		stat;
		cms_attribute		tmpattr;
		cms_attribute_value	tmpval;
		cms_access_entry	*alist;

		if ((stat = _DtCm_accessentry4_to_cmsaccesslist(cal->alist,
		    &alist)) == CSA_SUCCESS) {
			tmpattr.name.num = CSA_CAL_ATTR_ACCESS_LIST_I;
			tmpattr.name.name = CSA_CAL_ATTR_ACCESS_LIST;
			tmpattr.value = &tmpval;
			tmpval.item.access_list_value = NULL;
			tmpval.type = CSA_VALUE_ACCESS_LIST;

			if ((stat = _DtCm_copy_cms_attribute(attr, &tmpattr,
			    _B_TRUE)) == CSA_SUCCESS)
				attr->value->item.access_list_value = alist;
			else
				_DtCm_free_cms_access_entry(alist);
		}
		return (stat);
	}
}

static CSA_return_code
_GetCalendarName(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		return (_DtCm_copy_cms_attribute(attr,
			&cal->attrs[CSA_CAL_ATTR_CALENDAR_NAME_I], _B_TRUE));
	} else {
		attr->name.num = CSA_CAL_ATTR_CALENDAR_NAME_I;
		if (attr->name.name = strdup(CSA_CAL_ATTR_CALENDAR_NAME)) {
			return (_DtCm_set_string_attrval(cal->calendar,
				&attr->value, CSA_VALUE_STRING));
		} else {
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
	}
}

static CSA_return_code
_GetCalendarOwner(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	if (cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION) {
		return (_DtCm_copy_cms_attribute(attr,
			&cal->attrs[CSA_CAL_ATTR_CALENDAR_OWNER_I], _B_TRUE));
	} else {
		attr->name.num = CSA_CAL_ATTR_CALENDAR_OWNER_I;
		if (attr->name.name = strdup(CSA_CAL_ATTR_CALENDAR_OWNER)) {
			return (_DtCm_set_user_attrval(cal->owner,
				&attr->value));
		} else {
			return (CSA_E_INSUFFICIENT_MEMORY);
		}
	}
}

static CSA_return_code
_GetServerVersion(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	attr->name.num = CSA_X_DT_CAL_ATTR_SERVER_VERSION_I;
	if (attr->name.name = strdup(CSA_X_DT_CAL_ATTR_SERVER_VERSION))
		return (_DtCm_set_uint32_attrval((CSA_uint32)TABLEVERS,
			&attr->value));
	else
		return (CSA_E_INSUFFICIENT_MEMORY);
}

static CSA_return_code
_GetDataVersion(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	CSA_uint32	version;

	version = cal->fversion >= _DtCM_FIRST_EXTENSIBLE_DATA_VERSION ?
			cal->fversion : _DtCM_FIRST_EXTENSIBLE_DATA_VERSION - 1;

	attr->name.num = CSA_X_DT_CAL_ATTR_DATA_VERSION_I;
	if (attr->name.name = strdup(CSA_X_DT_CAL_ATTR_DATA_VERSION))
		return (_DtCm_set_uint32_attrval(version, &attr->value));
	else
		return (CSA_E_INSUFFICIENT_MEMORY);
}

static CSA_return_code
_GetProductId(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	attr->name.num = CSA_CAL_ATTR_PRODUCT_IDENTIFIER_I;
	if (attr->name.name = strdup(CSA_CAL_ATTR_PRODUCT_IDENTIFIER)) {
		return (_DtCm_set_string_attrval(_DtCM_PRODUCT_IDENTIFIER,
			&attr->value, CSA_VALUE_STRING));
	} else {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}
}

static CSA_return_code
_GetSupportedVersion(_DtCmsCalendar *cal, int index, cms_attribute *attr)
{
	attr->name.num = CSA_CAL_ATTR_VERSION_I;
	if (attr->name.name = strdup(CSA_CAL_ATTR_VERSION)) {
		return (_DtCm_set_string_attrval(_DtCM_SPEC_VERSION_SUPPORTED,
			&attr->value, CSA_VALUE_STRING));
	} else {
		return (CSA_E_INSUFFICIENT_MEMORY);
	}
}

/*
 * check validity of the access list
 */
static CSA_return_code
_CheckAccessList(cms_access_entry *alist, char *owner)
{
	char			*username, *ownername, *ptr, *ptr2, *ptr3;
	cms_access_entry	*head;
	CSA_return_code		stat = CSA_SUCCESS;
	boolean_t		checkroot, same;

	ownername = _DtCmGetPrefix(owner, '@');
	if (strcmp(ownername, _DTCMS_SUPER_USER) == 0)
		checkroot = _B_TRUE;
	else
		checkroot = _B_FALSE;

	for (; alist != NULL && stat == CSA_SUCCESS; alist = alist->next) {

		/* make sure calendar owner are granted CSA_OWNER_RIGHTS */

		username = _DtCmGetPrefix(alist->user, '@');
		if (checkroot == _B_TRUE) {
			if (strcmp(username, _DTCMS_SUPER_USER) == 0) {
				/* root on different machines are treated as
				 * different users
				 */
				if ((ptr = strchr(owner, '@')) &&
				    (ptr2 = strchr(alist->user, '@'))) {
					ptr++;

					if (ptr3 = strchr(ptr, '.'))
						same = _DtCmIsSamePath(ptr,
								       ++ptr2);
					else
						same = _DtCmIsSamePath(++ptr2,
								       ptr);

					if (same &&
					    !(alist->rights & CSA_OWNER_RIGHTS))
					{
						stat = CSA_E_INVALID_PARAMETER;
					}
				}
			}
		} else {

			if (strcmp(username, ownername) == 0 &&
			    (alist->rights & CSA_OWNER_RIGHTS) == 0) {
				stat = CSA_E_INVALID_PARAMETER;
			}
		}
		free(username);

		/* make sure there are no duplicates */
		for (head = alist->next; head != NULL && stat == CSA_SUCCESS;
		     head = head->next) {
			/* check for duplicate entries */
			if (strcmp(alist->user, head->user) == 0) {
				stat = CSA_E_INVALID_PARAMETER;
			}
		}
	} 

	free(ownername);
	return (stat);
}

