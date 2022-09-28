/* $XConsortium: cmcb.h /main/cde1_maint/1 1995/07/17 19:49:34 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CMCB_H
#define _CMCB_H

#pragma ident "@(#)cmcb.h	1.4 96/03/01 Sun Microsystems, Inc."

#ifndef SunOS
#include <rpc/types.h>
#endif

#include <rpc/rpc.h>

#ifdef __cplusplus
extern "C" {
#endif


#include "csa.h"
#include "cm.h"

/*
 * cal_attr_data - contain names of calendar attributes updated
 */

struct cmcb_cal_attr_data {
	CSA_uint32 num_names;
	cms_name *names;
};
typedef struct cmcb_cal_attr_data cmcb_cal_attr_data;

/*
 * add_entry_data - contain information of the deleted entry
 */

struct cmcb_add_entry_data {
	char *id;
};
typedef struct cmcb_add_entry_data cmcb_add_entry_data;

/*
 * delete_entry_data - contain information of the deleted entry
 */

struct cmcb_delete_entry_data {
	char *id;
	CSA_enum scope;
	time_t time;
};
typedef struct cmcb_delete_entry_data cmcb_delete_entry_data;

/*
 * update_entry_data - contain information of the updated entry
 * if new entry id is not resulted from the update, oldid will be
 * set to a NULL string ("").
 */

struct cmcb_update_entry_data {
	char *newid;
	char *oldid;
	CSA_enum scope;
	time_t time;
};
typedef struct cmcb_update_entry_data cmcb_update_entry_data;

/*
 * update_data - contains the update reason and specific information
 * about the update.  No additional data is provided for CSA_CALENDAR_LOGON,
 * CSA_CALENDAR_DELETED, and CSA_CALENDAR_ATTRIBUTE_UPDATED
 */
struct cmcb_update_data {
	CSA_flags	reason;
	union {
		cmcb_cal_attr_data	*cdata; /* data for cal attrs update */
		cmcb_add_entry_data	*adata; /* data for CSA_ENTRY_ADDED */
		cmcb_delete_entry_data	*ddata; /* data for CSA_ENTRY_DELETED */
		cmcb_update_entry_data	*udata; /* data for CSA_ENTRY_UPDATED */
	} data;
};
typedef struct cmcb_update_data cmcb_update_data;

struct cmcb_update_callback_args {
	char *calendar;
	char *user;
	cmcb_update_data data;
};
typedef struct cmcb_update_callback_args cmcb_update_callback_args;

#define	AGENTVERS_2 ((unsigned long)(2))

#if defined(__STDC__) || defined(__cplusplus)
#define	CMCB_UPDATE_CALLBACK ((unsigned long)(1))
extern  void * cmcb_update_callback_2(cmcb_update_callback_args *, CLIENT *);
extern  void * cmcb_update_callback_2_svc(cmcb_update_callback_args *, struct svc_req *);
extern int agentprog_2_freeresult(SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define	CMCB_UPDATE_CALLBACK ((unsigned long)(1))
extern  void * cmcb_update_callback_2();
extern  void * cmcb_update_callback_2_svc();
extern int agentprog_2_freeresult();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_cmcb_cal_attr_data(XDR *, cmcb_cal_attr_data*);
extern  bool_t xdr_cmcb_add_entry_data(XDR *, cmcb_add_entry_data*);
extern  bool_t xdr_cmcb_delete_entry_data(XDR *, cmcb_delete_entry_data*);
extern  bool_t xdr_cmcb_update_entry_data(XDR *, cmcb_update_entry_data*);
extern  bool_t xdr_cmcb_update_data(XDR *, cmcb_update_data*);
extern  bool_t xdr_cmcb_update_callback_args(XDR *, cmcb_update_callback_args*);

#else /* K&R C */
extern bool_t xdr_cmcb_cal_attr_data();
extern bool_t xdr_cmcb_add_entry_data();
extern bool_t xdr_cmcb_delete_entry_data();
extern bool_t xdr_cmcb_update_entry_data();
extern bool_t xdr_cmcb_update_data();
extern bool_t xdr_cmcb_update_callback_args();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#ifdef HPUX
#undef hpux
#define hpux
#endif
#endif

