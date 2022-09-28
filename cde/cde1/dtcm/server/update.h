/* $XConsortium: update.h /main/cde1_maint/1 1995/07/17 20:20:21 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _UPDATE_H
#define _UPDATE_H

#pragma ident "@(#)update.h	1.6 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "cmscalendar.h"

extern CSA_return_code _DtCmsUpdateCalAttrs(
			_DtCmsCalendar	*cal,
			uint		num_attrs,
			cms_attribute	*attrs);

extern CSA_return_code _DtCmsUpdateEntry(
			_DtCmsCalendar	*cal,
			char		*sender,
			uint		access,
			cms_key		*key,
			uint		num_attrs,
			cms_attribute	*attrs,
			cms_entry	**oldentry,
			cms_entry	**newentry);

extern CSA_return_code _DtCmsUpdateInstances(
			_DtCmsCalendar	*cal,
			char		*sender,
			uint		access,
			cms_key		*key,
			int		scope,
			uint		num_attrs,
			cms_attribute	*attrs,
			cms_entry	**oldentry,
			cms_entry	**newentry);

#endif
