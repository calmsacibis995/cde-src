/* $XConsortium: garbage.h /main/cde1_maint/1 1995/07/17 20:10:46 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _GARBAGE_H
#define _GARBAGE_H

#pragma ident "@(#)garbage.h	1.4 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

/*
**  Housekeeping mechanism to clean up the calendar log file.  It copies the
**  log file to a backup file, dumps the red/black tree to a temp file, copies
**  the temp file back to the original log file, and deletes the temp and
**  backup files.  Any errors encountered along the way abort the process.
**  The garbage collector runs at midnight every.
*/

extern void _DtCmsCollectOne(_DtCmsCalendar *cal);
extern CSA_return_code _DtCmsDumpDataV1(char *file, _DtCmsCalendar *cal);
extern CSA_return_code _DtCmsDumpDataV2(char *file, _DtCmsCalendar *cal);

#endif
