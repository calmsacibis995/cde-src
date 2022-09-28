
/*
 *      $XConsortium: util_ds.h /main/cde1_maint/2 1995/10/16 09:56:53 rcs $
 *
 * @(#)util_ds.h	1.1 19 Feb 1994      cde_app_builder/src/libAButil
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */

#ifndef _UTIL_DS_H_
#define _UTIL_DS_H_
/*
 * util_ds.h - LList and ASet ADTs
 */
#include <X11/Intrinsic.h>

typedef struct	_LListStruct	*LList;
typedef void	(*LListIterFn)(void *cl_data);

#define ASet		LList
#define ASetIterFn	LListIterFn


/*******************************
 * LList and ASet ADT operations
 *******************************/

extern LList	util_llist_create(
		    void
		);
extern void	util_llist_destroy(
		    LList	llist
		);
extern void	util_llist_insert_after(
		    LList	llist,
		    void	*cl_data
		);
extern LList	util_llist_find(
		    LList	llist,
		    void	*cl_data
		);
extern void	util_llist_delete(
		    LList	llist,
		    void	*cl_data
		);
extern void	util_llist_iterate(
		    LList	llist,
		    LListIterFn	fn
		);

#define util_aset_create	util_llist_create
#define util_aset_destroy	util_llist_destroy
#define util_aset_iterate	util_llist_iterate
#define util_aset_remove	util_llist_delete

extern void	util_aset_add(
		    ASet	aset,
		    void	*cl_data
		);

#endif /* _UTIL_DS_H_ */
