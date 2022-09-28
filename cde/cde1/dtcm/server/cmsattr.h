/*******************************************************************************
**
**  cmsattr.h
**
**  #pragma ident "@(#)cmsattr.h	1.4 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: cmsattr.h /main/cde1_maint/1 1995/07/14 23:25:03 drk $
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
#ifndef _CMSATTR_H
#define _CMSATTR_H

#include "ansi_c.h"
#include "cm.h"
#include "cmscalendar.h"

extern CSA_return_code _DtCmsUpdateSint32AttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmsUpdateUint32AttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmsUpdateStringAttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmsUpdateAccessListAttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmsUpdateReminderAttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmsUpdateDateTimeListAttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

extern CSA_return_code _DtCmsUpdateOpaqueDataAttrVal(
				cms_attribute_value *newval,
				cms_attribute_value **attrval);

#endif
