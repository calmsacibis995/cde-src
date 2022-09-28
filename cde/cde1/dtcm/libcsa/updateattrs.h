/* $XConsortium: updateattrs.h /main/cde1_maint/1 1995/07/17 20:04:44 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _UPDATEATTRS_H
#define _UPDATEATTRS_H

#pragma ident "@(#)updateattrs.h	1.7 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "csa.h"
#include "cm.h"
#include "nametbl.h"

extern CSA_return_code _DtCmUpdateAttribute(
			cms_attribute	*from,
			cms_attribute	*to);

extern CSA_return_code _DtCmUpdateAccessListAttrVal(
			cms_attribute_value *newval,
			cms_attribute_value **attrval);

extern CSA_return_code _DtCmUpdateSint32AttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmUpdateStringAttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmUpdateReminderAttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmUpdateDateTimeListAttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmUpdateOpaqueDataAttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

#endif
