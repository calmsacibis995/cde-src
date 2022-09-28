/* $XConsortium: callback.h /main/cde1_maint/3 1995/10/10 13:32:13 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CALLBACK_H
#define _CALLBACK_H

#pragma ident "@(#)callback.h	1.10 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "rtable4.h"

typedef struct __DtCmsRegistrationInfo {
	char	*client;
	int	types;
	u_long	prognum;
	u_long	versnum;
	u_long	procnum;
	int	pid;
	struct	__DtCmsRegistrationInfo *next;
} _DtCmsRegistrationInfo;

extern _DtCmsRegistrationInfo *_DtCmsMakeRegistrationInfo(
				char *client,
				int types,
				u_long prognum,
				u_long versnum,
				u_long procnum,
				int pid);

extern void _DtCmsFreeRegistrationInfo(_DtCmsRegistrationInfo *w);

extern _DtCmsRegistrationInfo *_DtCmsGetRegistration(
				_DtCmsRegistrationInfo **rlist,
				char *client,
				u_long prognum,
				u_long versnum,
				u_long procnum,
				int pid);

extern _DtCmsRegistrationInfo *_DtCmsRemoveRegistration(
				_DtCmsRegistrationInfo *rlist,
				_DtCmsRegistrationInfo *rinfo);

extern _DtCmsRegistrationInfo *_DtCmsCheckRegistrationList(
				_DtCmsRegistrationInfo *rlist);

extern _DtCmsRegistrationInfo * _DtCmsDoV1CbForV4Data(
				_DtCmsRegistrationInfo	*rlist,
				char			*source,
				int			pid,
				cms_key			*key1,
				cms_key			*key2);

extern _DtCmsRegistrationInfo *_DtCmsDoV1Callback(
					_DtCmsRegistrationInfo *rlist,
					char *source,
					int pid,
					Appt_4 *a);

extern _DtCmsRegistrationInfo *_DtCmsDoOpenCalCallback(
					_DtCmsRegistrationInfo *rlist,
					char *cal,
					char *user,
					int pid);

extern _DtCmsRegistrationInfo *_DtCmsDoRemoveCalCallback(
					_DtCmsRegistrationInfo *rlist,
					char *cal,
					char *user,
					int pid);

extern _DtCmsRegistrationInfo *_DtCmsDoUpdateCalAttrsCallback(
					_DtCmsRegistrationInfo	*rlist,
					char			*cal,
					char			*user,
					uint			num_attrs,
					cms_attribute		*attrs,
					int			pid);

extern _DtCmsRegistrationInfo *_DtCmsDoInsertEntryCallback(
					_DtCmsRegistrationInfo *rlist,
					char *cal,
					char *source,
					long id,
					int pid);

extern _DtCmsRegistrationInfo *_DtCmsDoDeleteEntryCallback(
					_DtCmsRegistrationInfo *rlist,
					char *cal,
					char *source,
					long id,
					int scope,
					time_t time,
					int pid);

extern _DtCmsRegistrationInfo *_DtCmsDoUpdateEntryCallback(
					_DtCmsRegistrationInfo *rlist,
					char *cal,
					char *source,
					long newid,
					long oldid,
					int scope,
					long time,
					int pid);

extern void _DtCmsListRegistration(_DtCmsRegistrationInfo *rlist,
				      char *cal);


#endif
