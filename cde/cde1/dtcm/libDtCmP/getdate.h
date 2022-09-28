/*******************************************************************************
**
**  getdate.h
**
**  #pragma ident "@(#)getdate.h	1.9 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: getdate.h /main/cde1_maint/1 1995/07/14 23:18:48 drk $
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

#ifndef _GETDATE_H
#define _GETDATE_H

#ifdef USG
struct timeb
{
        time_t  time;
        unsigned short millitm;
        short   timezone;
        short   dstflag;
};
#else
#include <sys/timeb.h>
#endif
#include <time.h>
#include "ansi_c.h"

#define BOT_YEAR	1970
#define EOT_YEAR	2037
#define DATE_PARSE	-1	/* date simply did not parse */
#define DATE_BBOT	-2	/* date was before beginning of time */
#define	DATE_AEOT	-3	/* date was after end of time */
#define DATE_BMONTH	-4	/* date had a bad month number */
#define DATE_BDAY	-5	/* date had a bad day number */
#define DATE_BMIN	-6	/* date had a bad minute number */
#define DATE_BHOUR	-7	/* date had a bad hour number */
#define DATE_CONV	-8	/* date converted poorly for am/pm/24hr */

extern time_t		dateconv	(int, int, int, int, int, int, int, int, int);
extern time_t		dayconv		(int, int, time_t);
extern time_t		timeconv	(int, int, int, int);
extern time_t		monthadd	(time_t, time_t);
extern time_t		daylcorr	(time_t, time_t);
extern time_t		cm_getdate	(char*, struct timeb *);


#endif
