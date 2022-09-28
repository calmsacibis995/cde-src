/* $XConsortium: convert4-3.h /main/cde1_maint/1 1995/07/17 19:54:14 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CONVERT4_3_H
#define _CONVERT4_3_H

#pragma ident "@(#)convert4-3.h	1.5 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

/* convert4-3.h:  conversion routines for rpc.cmsd version 4 to
   rpc.cmsd version 3 data types
*/

extern Table_Res_3 *_DtCm_tableres4_to_tableres3(Table_Res_4*);
extern Table_Status_3 _DtCm_tablestat4_to_tablestat3(Table_Status_4);
extern Registration_Status_3 _DtCm_regstat4_to_regstat3(Registration_Status_4);
extern Access_Status_3 _DtCm_accstat4_to_accstat3(Access_Status_4);
extern Access_Args_3 *_DtCm_accargs4_to_accargs3(Access_Args_4*);
extern Uid_3 *_DtCm_uid4_to_uid3(Uid_4*);
extern Range_3 *_DtCm_range4_to_range3(Range_4*);
extern Appt_3 *_DtCm_appt4_to_appt3(Appt_4*);
extern void _DtCm_id4_to_id3(Id_4*, Id_3*);
extern Access_Entry_3 *_DtCm_acclist4_to_acclist3(Access_Entry_4*);
extern Keyrange_3 *_DtCm_keyrange4_to_keyrange3(Keyrange_4*);
extern Uid_3 *_DtCm_uidopt4_to_uid3(Uidopt_4*);
extern void _DtCm_free_attr3(Attribute_3 *a);
extern void _DtCm_free_appt3(Appt_3 *appts);

#endif
