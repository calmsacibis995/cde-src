/* $XConsortium: cmsdata.h /main/cde1_maint/2 1995/09/06 08:19:52 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CMSDATA_H
#define _CMSDATA_H

#pragma ident "@(#)cmsdata.h	1.12 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "nametbl.h"
#include "cm.h"

/* standard calendar attr name and entry attr name table */
extern _DtCmNameTable *_DtCm_cal_name_tbl;
extern _DtCmNameTable *_DtCm_entry_name_tbl;

extern void _DtCm_init_hash();

extern cms_entry *_DtCm_make_cms_entry(_DtCmNameTable *tbl);

extern CSA_return_code _DtCm_copy_cms_entry(
				cms_entry *e,
				cms_entry **e_r);

extern void _DtCm_free_cms_entry(cms_entry *entry);

extern void _DtCm_free_cms_entries(cms_entry *entry);

extern CSA_return_code _DtCmGrowAttrArray(
			CSA_uint32	*num_attr,
			cms_attribute	**attrs,
			cms_attribute	*attr);

#endif
