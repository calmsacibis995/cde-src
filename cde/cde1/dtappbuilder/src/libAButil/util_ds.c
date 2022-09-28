/*
 *      $XConsortium: util_ds.c /main/cde1_maint/2 1995/10/16 17:07:40 rcs $
 *
 * @(#)util_ds.c	1.3 23 Feb 1994      cde_app_builder/src/libAButil
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


#include <X11/Intrinsic.h>
#include <ab_private/util_ds.h>

/***************************************************
 * Linked List and Active Set ADT implementations
 ***************************************************/

struct _LListStruct {
    void		*value;
    struct _LListStruct	*next;
};

static void		llist_delete_next(
			    LList	llist
			);

extern LList
util_llist_create(
    void
)
{
    LList tmp = (LList) XtMalloc(sizeof(struct _LListStruct));

    tmp->next = NULL;
    return(tmp);
}

extern void
util_llist_destroy(
    LList	llist
)
{
    while (llist != NULL)
    {
	LList	tmp = llist;

	llist = llist->next;
	XtFree((char *)tmp);
    }
}

extern void
util_llist_insert_after(
    LList	llist,
    void	*cl_data
)
{
    LList	tmp = util_llist_create();

    tmp->value  = cl_data;
    tmp->next   = llist->next;
    llist->next = tmp;
}

extern LList
util_llist_find(
    LList	llist,
    void	*cl_data
)
{
    ASet	prev;
    ASet	trav;

    for (prev = llist, trav = llist->next; trav != NULL;
			    prev = trav, trav = trav->next)
	if (trav->value == cl_data)
	    return(prev);
    return(NULL);
}

extern void
util_llist_delete(
    LList	llist,
    void	*cl_data
)
{
    LList	tmp = util_llist_find(llist, cl_data);

    if (tmp != NULL)
	llist_delete_next(tmp);
}

static void
llist_delete_next(
    LList	llist
)
{
    LList	tmp = llist->next;

    if (tmp != NULL)
    {
	llist->next = tmp->next;
	XtFree((char *)tmp);
    }
}

extern void
util_llist_iterate(
    LList	llist,
    LListIterFn	fn
)
{
    for (llist = llist->next; llist != NULL; llist = llist->next)
	(*fn)(llist->value);
}

extern void
util_aset_add(
    ASet	aset,
    void	*cl_data
)
{
    if (util_llist_find(aset, cl_data) == NULL)
	util_llist_insert_after(aset, cl_data);
}
