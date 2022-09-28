/* $XConsortium: insert.h /main/cde1_maint/1 1995/07/17 20:11:22 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _INSERT_H
#define _INSERT_H

#pragma ident "@(#)insert.h	1.4 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "cmscalendar.h"

extern CSA_return_code _DtCmsInsertEntry(
			_DtCmsCalendar	*cal,
			cms_entry	*entry);

extern CSA_return_code _DtCmsInsertEntryAndLog(
			_DtCmsCalendar	*cal,
			cms_entry	*entry);

#endif
