/* $XConsortium: cmfns.h /main/cde1_maint/1 1995/07/17 19:45:27 drk $ */
/*	@(#)cmfns.h 1.7 97/01/31 SMI	*/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */


#ifndef _CMFNS_H
#define _CMFNS_H

#ifdef FNS

#include "dtfns.h"

extern const char *	CMFNS_CALENDAR_NAME;
extern const char *	CMFNS_CALENDAR_TYPE;
extern const char *	CMFNS_CALENDAR_ADDR_TYPE;
extern const char 	CMFNS_HOST_SEP;
extern const char *	CMFNS_USER_STR;
extern const char *	CMFNS_HOST_STR;
extern const char *	CMFNS_SERVICE_STR;
extern const char *	CMFNS_SERVICE_STR2;
extern const char *	CMFNS_CAL_STR;
extern const char *	CMFNS_CAL_STR2;
extern const char *	FNS_FILE;

#define	CMFNS_E_NAME_NOT_BOUND	50
#define	CMFNS_MAXNAMELEN	256

#ifdef FNS_DEMO

int cmfns_name_from_file(const char *path, char *name, int len);

#endif /* FNS_DEMO */

extern int cmfns_use_fns(Props *);
extern int cmfns_lookup_calendar(const char *name, char *addr_buf,
				 int addr_size);
extern int cmfns_register_calendar(const char *name, const char *calendar);

extern char *
cmfns_make_calname(const char *defcal, char **caladdr);

extern int
cmfns_set_default_calendar(const char* defcal, char **calname, char **caladdr);

extern int
cmfns_read_name_from_file(char *filename, char *name_buf, int nsize);

#endif /* FNS */

#endif /* _CMFNS_H */

