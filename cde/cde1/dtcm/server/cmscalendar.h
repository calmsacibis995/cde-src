/* $XConsortium: cmscalendar.h /main/cde1_maint/2 1995/09/06 08:26:07 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CMSCALENDAR_H
#define _CMSCALENDAR_H

#pragma ident "@(#)cmscalendar.h	1.15 96/05/13 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "csa.h"
#include "cm.h"
#include "nametbl.h"
#include "tree.h"
#include "list.h"
#include "data.h"
#include "reminder.h"
#include "rtable4.h"
#include "callback.h"
#include "log.h"

#ifdef MT
#include <pthread.h>
#endif

#define	APPT_TREE(info)			((Rb_tree *) ((info)->tree))
#define	REPT_LIST(info)			((Hc_list *) ((info)->list))
#define	APPT_KEY(p_appt)		((Appt_4 *) (p_appt))->appt_id.key
#define	APPT_TICK(p_appt)		((Appt_4 *) (p_appt))->appt_id.tick

typedef CSA_return_code (*_DtCmGetAttrFunc)();

typedef	struct __DtCmsCalendar {
	char		*owner;
	char		*calendar;
	time_t	        modtime;        /* time callog file was last modified */
        long            blksize;        /* size of callog file during last
                                         * modification */

	_DtCmNameTable	*cal_tbl;
	_DtCmNameTable	*entry_tbl;
	int		*types;		/* type associated with entry attrs */
	int		num_entry_attrs; /* number of entry attrs associated
					  * with this calendar */
	boolean_t	hashed;		/* true if file converted to hashed
					 * format */
	int		fversion;
	long		lastkey;
	boolean_t	modified;	/* if true, do garbage collection */
	Rb_tree		*tree;		/* for single appointments */
	Hc_list		*list;		/* for repeating appointments */
	Rm_que		*rm_queue;	/* active reminder queue, version 1 */
	_DtCmsRemQueue	*remq;		/* reminder queue, version 4 */
	Access_Entry_4	*r_access;	/* read access, version 1 */
	Access_Entry_4	*w_access;	/* write access, version 1 */
	Access_Entry_4	*d_access;	/* delete access, version 1 */
	Access_Entry_4 	*x_access;	/* exec access, version 1 */
	Access_Entry_4	*alist;		/* combined v1 access list */
	CSA_uint32	num_attrs;	/* number of calendar attrs,version 4 */
	cms_attribute 	*attrs;		/* calendar attributes, version 4 */
	_DtCmGetAttrFunc *getattrfuncs;	/* array of function ptr to get attrs */
	_DtCmsRegistrationInfo *rlist;	/* client registration list */
	boolean_t	checkowner;
	struct __DtCmsCalendar *next;

#ifdef MT /* locks for protecting calendar data */
        pthread_mutex_t list_lock;      /* protects next pointer */
        pthread_mutex_t synch_lock;     /* protects following fields */
        pthread_cond_t  wake;           /* wake waiting threads */
        int             wait_count;     /* number of threads waiting in queue */
        int             deleted;        /* last thread must delete calendar */
        int             owned;          /* calendar data is being accessed */
        int             incomplete;     /* calendar was not loaded completely */
#endif
} _DtCmsCalendar;

extern CSA_return_code _DtCmsInitCalendar(_DtCmsCalendar *cal, char *owner);

extern void _DtCmsUnlockCalendar(_DtCmsCalendar *cal);

extern void _DtCmsFreeCalendar(_DtCmsCalendar *cal, boolean_t incomplete);

extern CSA_return_code _DtCmsSetFileVersion(_DtCmsCalendar *cal, int version);

extern CSA_return_code _DtCmsGetCalendarByName(char *target, _DtCmsCalendar **cal);

extern CSA_return_code _DtCmsAddCalendarToList(char *target, _DtCmsCalendar **cal);

extern CSA_return_code _DtCmsFindCalendarInList(char *target, _DtCmsCalendar **cal);

extern CSA_return_code _DtCmsLoadCalendar(_DtCmsCalendar *cal);

extern CSA_return_code _DtCmsInsertEntry4Parser(_DtCmsCalendar *cal, cms_entry *entry);

extern void _DtCmsSetAccess4Parser(_DtCmsCalendar *cal, Access_Entry_4 *list, int type);

extern void _DtCmsSetCalendarAttrs4Parser(_DtCmsCalendar *cal, int len, cms_attribute *attrs);

extern void _DtCmsGenerateKey(_DtCmsCalendar *cal, long *key);

extern CSA_return_code _DtCmsEnumerateUp(_DtCmsCalendar *cal, _DtCmsEnumerateProc doit);

extern void _DtCmsEnumerateDown(_DtCmsCalendar *cal, _DtCmsEnumerateProc doit);

extern CSA_return_code _DtCmsRbToCsaStat(Rb_Status rb_stat);

extern CSA_return_code _DtCmsGetCalAttrsByName(_DtCmsCalendar *cal, CSA_uint32 num_names, 
                                               cms_attr_name *names, CSA_uint32 *num_attrs_r, 
					       cms_attribute **attrs_r);

extern CSA_return_code _DtCmsGetAllCalAttrs(_DtCmsCalendar *cal, CSA_uint32 *num_attrs_r,
					    cms_attribute **attrs_r, boolean_t returnall);

extern CSA_return_code _DtCmsGetCalAttrNames(_DtCmsCalendar *cal, CSA_uint32 *num_names_r,
					     cms_attr_name **names_r);

extern void _DtCmsFreeCmsAttrNames(CSA_uint32 num, cms_attr_name *names);

extern CSA_return_code _DtCmsUpdateCalAttributesAndLog(_DtCmsCalendar *cal, CSA_uint32 numsrc,
						       cms_attribute *srcattrs, CSA_flags access);

extern CSA_return_code _DtCmsV5TransactLog(_DtCmsCalendar *cal, cms_entry *e, _DtCmsLogOps op);

extern void _DtCmsMarkCheckOwner(_DtCmsCalendar *cal);

#endif
