/* $XConsortium: convert2-4.h /main/cde1_maint/1 1995/07/17 19:52:23 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CONVERT2_4_H
#define _CONVERT2_4_H

#pragma ident "@(#)convert2-4.h	1.5 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

/* convert2-4.h:  conversion routines for rpc.cmsd version 2 to
   rpc.cmsd version 4 data types
*/

extern Table_Args_4 *_DtCm_tableargs2_to_tableargs4(Table_Args_2*);
extern Table_Args_4 *_DtCm_tabledelargs2_to_tabledelargs4(Table_Args_2*, Options_4);
extern Registration_4 *_DtCm_reg2_to_reg4(Registration_2*);
extern Access_Args_4 *_DtCm_accargs2_to_accargs4(Access_Args_2*);
extern Table_Res_4 *_DtCm_tableres2_to_tableres4(Table_Res_2*);
extern Table_Status_4 _DtCm_tablestat2_to_tablestat4(Table_Status_2);
extern Access_Status_4 _DtCm_accstat2_to_accstat4(Access_Status_2);
extern Registration_Status_4 _DtCm_regstat2_to_regstat4(Registration_Status_2);

#endif
