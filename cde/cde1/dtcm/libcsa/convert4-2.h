/* $XConsortium: convert4-2.h /main/cde1_maint/1 1995/07/17 19:53:38 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CONVERT4_2_H
#define _CONVERT4_2_H

#pragma ident "@(#)convert4-2.h	1.5 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

/* convert4-2.h:  conversion routines for rpc.cmsd version 4 to
   rpc.cmsd version 2 data types
*/

extern Table_Res_2 *_DtCm_tableres4_to_tableres2(Table_Res_4 *);
extern Table_Status_2 _DtCm_tablestat4_to_tablestat2(Table_Status_4);
extern Registration_Status_2 _DtCm_regstat4_to_regstat2(Registration_Status_4);
extern Access_Status_2 _DtCm_accstat4_to_accstat2(Access_Status_4);
extern Access_Args_2 *_DtCm_accargs4_to_accargs2(Access_Args_4*);
extern Uid_2 *_DtCm_uid4_to_uid2(Uid_4*);
extern Range_2 *_DtCm_range4_to_range2(Range_4*);
extern Appt_2 *_DtCm_appt4_to_appt2(Appt_4*);
extern void _DtCm_id4_to_id2(Id_4*, Id_2*);
extern Access_Entry_2 *_DtCm_acclist4_to_acclist2(Access_Entry_4*);
extern Uid_2 *_DtCm_uidopt4_to_uid2(Uidopt_4*);
extern void _DtCm_free_attr2(Attribute_2 *a);
extern void _DtCm_free_appt2(Appt_2 *a);

#endif
