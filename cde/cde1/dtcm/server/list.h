/* $XConsortium: list.h /main/cde1_maint/2 1995/10/10 13:34:05 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _LIST_H
#define _LIST_H

#pragma ident "@(#)list.h	1.8 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "data.h"
#include "rerule.h"

typedef struct lnode {
	struct lnode	*llink;
	struct lnode	*rlink;
	caddr_t		data;
	time_t		lasttick;
	int		duration;
	RepeatEvent	*re;
} List_node;

typedef struct {
	List_node	*root;
	caddr_t	private;	/* for internal tool state */
} Hc_list;

typedef int(*Destroy_proc)(caddr_t);

#define	hc_lookup_next(p_node)		(p_node)->rlink
#define	hc_lookup_previous(p_node)	(p_node)->llink

extern Rb_Status	hc_check_list(Hc_list*);
extern Hc_list*		hc_create(_DtCmsGetKeyProc, _DtCmsCompareProc);
extern List_node	*hc_delete(Hc_list*, caddr_t key);
extern List_node	*hc_delete_node(Hc_list*, List_node*);
extern void		hc_destroy(Hc_list*, Destroy_proc);
extern void		hc_enumerate_down(Hc_list*, _DtCmsEnumerateProc);
extern Rb_Status	hc_enumerate_up(Hc_list*, _DtCmsEnumerateProc);
extern Rb_Status	hc_insert(Hc_list*, caddr_t data, caddr_t key,
					RepeatEvent *re, List_node **node_r);
extern caddr_t		hc_lookup(Hc_list*, caddr_t key);
extern caddr_t		hc_lookup_largest(Hc_list*);
extern caddr_t		hc_lookup_next_larger(Hc_list*, caddr_t key);
extern caddr_t		hc_lookup_next_smaller(Hc_list*, caddr_t key);
extern caddr_t		hc_lookup_smallest(Hc_list*);
extern int		hc_size	(Hc_list*);
extern List_node	*hc_lookup_node(Hc_list*, caddr_t key);
extern Hc_list*		hc_create(_DtCmsGetKeyProc, _DtCmsCompareProc);
extern Rb_Status	hc_insert_node(Hc_list *, List_node *, caddr_t key);

#endif
