/* $XConsortium: cmsmatch.h /main/cde1_maint/2 1995/10/10 13:33:03 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CMSMATCH_H
#define _CMSMATCH_H

#pragma ident "@(#)cmsmatch.h	1.7 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "nametbl.h"

extern boolean_t _DtCmsMatchAttributes(
			cms_entry *entry,
			CSA_uint32 num_attrs,
			cms_attribute *attrs,
			CSA_enum *ops);

#endif
