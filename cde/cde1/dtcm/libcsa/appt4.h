/* $XConsortium: appt4.h /main/cde1_maint/1 1995/07/17 19:47:41 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _APPT4_H
#define _APPT4_H

#pragma ident "@(#)appt4.h	1.6 96/11/12 Sun Microsystems, Inc."

/*
 * this file contains allocate and free routines for v4 data structures
 */

#include "rtable4.h"
#include "ansi_c.h"

extern Appt_4 *_DtCm_make_appt4();

extern Appt_4 *_DtCm_copy_one_appt4(Appt_4 *a4);

extern Appt_4 *_DtCm_copy_appt4(Appt_4 *a4);

extern Appt_4 *_DtCm_copy_semiprivate_appt4(Appt_4 *original);

extern Except_4 *_DtCm_copy_excpt4(Except_4 *e4);

extern Abb_Appt_4 *_DtCm_appt_to_abbrev4(Appt_4 *original);

extern Abb_Appt_4 *_DtCm_appt_to_semiprivate_abbrev4(Appt_4 *original);

extern void _DtCm_free_appt4(Appt_4 *a);

extern void _DtCm_free_abbrev_appt4(Abb_Appt_4 *a);

extern Attribute_4 * _DtCm_make_attr4();

extern void _DtCm_free_attr4(Attribute_4 *a);

extern Reminder_4 *_DtCm_copy_reminder4(Reminder_4 *r4);

extern void _DtCm_free_reminder4(Reminder_4 *r);

extern void _DtCm_free_keyentry4(Uid_4 *k);

extern Access_Entry_4 *_DtCm_make_access_entry4(char *who, int perms);

extern Access_Entry_4 *_DtCm_copy_access_list4(Access_Entry_4 *l4);

extern void _DtCm_free_access_list4(Access_Entry_4 *l);

extern void _DtCm_free_excpt4(Except_4 *e);

extern char * _DtCm_get_default_what_string();

#endif
