/*******************************************************************************
**
**  cmfns.c
**
**  $XConsortium: cmfns.c /main/cde1_maint/1 1995/07/14 23:18:18 drk $
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

#pragma ident "@(#)cmfns.c	1.9 97/01/31 Sun Microsystems, Inc."

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved
 */

#ifdef FNS
#ifndef PTHREAD
#error "(1) FNS requires POSIX threads!"
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <pthread.h>

#include "cm_tty.h"
#include "debug.h"
#include "props.h"
#include "dtfns.h"
#include "cmfns.h"

const char *	CMFNS_CALENDAR_NAME	=	"calendar";
const char *	CMFNS_CALENDAR_TYPE	=	"onc_calendar";
const char *	CMFNS_CALENDAR_ADDR_TYPE=	"onc_cal_str";
const char 	CMFNS_HOST_SEP		=	'@';
const char *	CMFNS_USER_STR		=	"user/";
const char *	CMFNS_HOST_STR		=	"host/";
const char *	CMFNS_SERVICE_STR	=	"/_service/";
const char *	CMFNS_SERVICE_STR2	=	"/service/";
const char *	CMFNS_CAL_STR		=	"/_service/calendar";
const char *	CMFNS_CAL_STR2		=	"/service/calendar";
const char *	FNS_FILE		=	".FNSName";

static const char *	ss		=	"%s%s";

static pthread_once_t	cmfns_once_init = PTHREAD_ONCE_INIT;
static boolean_t	fns_available = _B_FALSE;

/*
 * Done only once (by pthread_once()).
 */
void
cmfns_once_proc()
{
  if (dtfns_available() == -1) {
    fns_available = _B_FALSE;
  } else {
    fns_available = _B_TRUE;
  }
  return;
}

/*
 * Returns 1 if we can use FNS, else 0
 *
 * The first call to this routine can be a little costly.  Subsequent calls
 * are very cheap.  So avoid calling this routine at startup.
 */
int
cmfns_use_fns(Props *p)
{
  boolean_t	use_fns = _B_FALSE;

  pthread_once(&cmfns_once_init, cmfns_once_proc);

  /* get 'UseFNS' property dynamically */
  if (fns_available) {
    use_fns = get_int_prop(p, CP_USEFNS);
  }
  return (use_fns);
}

#define strtail(s1, s2) \
	(((tmp = strstr(s1, s2)) == 0) ? 0 : \
		((tmp == (s1 + (strlen(s1) - strlen(s2)))) ? 1 : 0))

static int is_old_cm_format(const char *str)
{
	return (strchr(str, CMFNS_HOST_SEP) != 0);
}

/* figure out full name of calendar */
static char *
cmfns_get_fns_fullname(const char* fns_name)
{
  char	*	str;
  char 	*	tmp;
  char	*	fullname = NULL;

  if (fns_name != NULL) {

    fullname = (char *)malloc(CMFNS_MAXNAMELEN);
    if (fullname != NULL) {

      /* If no FNS separator, assume it is user */
      if ((str = strchr(fns_name, DTFNS_SEPARATOR)) == NULL) {
	sprintf(fullname, "%s%s%s", CMFNS_USER_STR, 
		fns_name, CMFNS_CAL_STR);
      } else if (strtail(fns_name, CMFNS_CAL_STR) ||
		 strtail(fns_name, CMFNS_CAL_STR2)) {
	/* already fullname */
	strcpy(fullname, fns_name);
      } else if (strtail(fns_name, CMFNS_SERVICE_STR) ||
		 strtail(fns_name, CMFNS_SERVICE_STR2)) {
	sprintf(fullname, ss, fns_name, CMFNS_CALENDAR_NAME);
      } else {
	sprintf(fullname, ss, fns_name, CMFNS_CAL_STR);
      }
    }
  }
  return (fullname);
}

/*
 *
 *	Get a calendar address from FNS
 *
 *      'name' may  be an absolute FNS name:
 *              user/smith
 *              org/ssi.eng/user/smith
 *              user/smith/service
 *
 *	'name' may be an FNS shorthand name:
 *		dipol
 *
 *	'name' may be a calendar address:
 *		dipol@sidewinder
 *	In which case it is copied unmodified into addr_buf.
 *
 *      'buf' is a buffer provided by the caller in which the calendar address
 *      is placed.
 *
 *      'size' is the size of 'buf'
 *
 *
 * Returns
 *
 *	-1	Name not found / Error
 *	0	FNS not available
 *	1	Success
 *
 */
int
cmfns_lookup_calendar(const char *fns_name, char *cal_addr, int addr_size)
{
  char	*	fullname;
  int		rcode = -1;

  DP(("cmfns_lookup_calendar: Looking up %s\n", name));

  /* do nothing if the fns_name is of old format */
  if (is_old_cm_format(fns_name)) {
    strncpy(cal_addr, fns_name, addr_size);
    cal_addr[addr_size-1] = '\0';
    return(1);
  }

  fullname = cmfns_get_fns_fullname(fns_name);

  rcode = dtfns_lookup_calendar(fullname, 
				CMFNS_CALENDAR_TYPE,
				CMFNS_CALENDAR_ADDR_TYPE,
				cal_addr, addr_size);
  if (rcode <= 0) {
    free(fullname);
    return rcode;
  }

  if (strchr(cal_addr, CMFNS_HOST_SEP) == NULL) {
    /*
     * Just the location (host) is bound in FNS.  Pull
     * the name of the object (user) out of the FNS
     * name
     */
    char	*	tmp_addr = strdup(cal_addr);
    char	*	ptr;

    if ((ptr = strstr(fullname, CMFNS_CAL_STR)) == NULL &&
	(ptr = strstr(fullname, CMFNS_CAL_STR2)) == NULL) {
      free(tmp_addr);
      free(fullname);
      return -1;
    }
    *ptr = '\0';
    while (*ptr != DTFNS_SEPARATOR) {
      ptr--;
    }
    sprintf(cal_addr, "%s@%s", ptr + 1, tmp_addr);
    free(tmp_addr);
    free(fullname);
  }
  DP(("cmfns_lookup_calendar: FNS Lookup complete. address=%s", cal_buf));

  return rcode;
}

/*
* Register a calendar location in FNS.
 */
int
cmfns_register_calendar(const char *username, const char *location)

{
  int		rcode = -1;
  char		cal_addr[CMFNS_MAXNAMELEN];
  char	*	ptr;
  char	*	fullname;
  char	*	user;

  user = strdup(username);

  if ((ptr = strchr(user, CMFNS_HOST_SEP)) != NULL) {
    *ptr = '\0';
  }

  DP(("register_calendar: Binding %s to %s\n", location, buf));
  fullname = cmfns_get_fns_fullname(user);
  free(user);
  rcode = dtfns_register_calendar(fullname, 
				  CMFNS_CALENDAR_TYPE, 
				  CMFNS_CALENDAR_ADDR_TYPE,
				  location); 
  free(fullname);
  return (rcode);
}

/* Sets the calendar's name and address using 'default' (a user@host) */
int
cmfns_set_default_calendar(const char* defcal, char **calname, char **caladdr)
{

  /*
   * try to look up calendar in namespace using user's name.
   * if found, use that, otherwise, use information
   * from properties
   */

  /* extract user name from user's credentials */
  char		user_name[CMFNS_MAXNAMELEN];
  char		tmp[CMFNS_MAXNAMELEN];
  char	*	user = cm_target2name((char *)defcal);

  user_name[0] = '\0';

  if (strcmp(user, "root") == 0) {
    /* actually a machine's calendar */
    free(user);
    user = cm_target2location((char *)defcal);
    sprintf(user_name, ss, CMFNS_HOST_STR, user);
  } else {
    sprintf(user_name, ss, CMFNS_USER_STR, user);
  }
  free(user);
	
  if (cmfns_lookup_calendar(user_name, tmp, 0) > 0) {
    *calname = cm_strdup(user_name);
    *caladdr = cm_strdup(tmp);
    return (0);  /* already exists */
  } else {
    *calname = cm_strdup((char *)defcal);
    *caladdr = cm_strdup((char *)defcal);
    return (1);
  }
}

char *
cmfns_make_calname(const char *defcal, char **caladdr)
{
  char	*	calname;
  char		tmp[CMFNS_MAXNAMELEN];

  /* calendar name is in old format */
  if (is_old_cm_format(defcal)) {
    if (caladdr) {
      *caladdr = cm_strdup((char *)defcal);
    }
    return (cm_strdup((char *)defcal));
  }

  if (strchr(defcal, DTFNS_SEPARATOR) == NULL) {
    sprintf(tmp, ss, CMFNS_USER_STR, defcal);
    calname = cm_strdup(tmp);
  } else {
    calname = cm_strdup((char *)defcal);
  }
  
  if (caladdr) {
    if (cmfns_lookup_calendar(calname, tmp, 0) > 0) {
      *caladdr = cm_strdup(tmp);
    } else {
      *caladdr = cm_strdup(calname);
    }
  }
  return (calname);
}

/* Read a single string name from a file */

int
cmfns_read_name_from_file(char *filename, char *name_buf, int nsize)
{
  int		results = 0;
  FILE 	*	fp = NULL;
  char	*	ptr;

  if ((fp = fopen(filename, "r")) != NULL) {
    if (fgets(name_buf, nsize, fp) != NULL) {

      /* Make sure name is null terminated */
      if (ptr = strrchr(name_buf, '\n')) {
	*ptr = '\0';
      } else {
	name_buf[nsize-1] = '\0';
      }
    }
    fclose(fp);
  }
  return (results);
}

#endif /* FNS */
