/* $XConsortium: tree.h /main/cde1_maint/1 1995/07/17 20:19:45 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _TREE_H
#define _TREE_H

#pragma ident "@(#)tree.h	1.5 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "data.h"

/*
**  2-3-4 tree, a.k.a. red-black tree
*/

typedef enum {red=0, black=1} Color;

typedef struct node {
	struct node *llink;
	struct node *rlink;
	Color	color;
	caddr_t	data;
} Tree_node;

typedef struct {
	Tree_node *root;
	caddr_t private;	/* for internal tool state */
} Rb_tree;

extern Rb_tree* rb_create(_DtCmsGetKeyProc, _DtCmsCompareProc);

extern void rb_destroy(Rb_tree*, _DtCmsEnumerateProc); 

extern int rb_size(Rb_tree*t);

extern Rb_Status rb_insert(Rb_tree*, caddr_t data, caddr_t key); 

extern Rb_Status rb_insert_node(Rb_tree*, Tree_node*, caddr_t key);

extern Tree_node * rb_delete(Rb_tree*, caddr_t key);

extern caddr_t rb_lookup(Rb_tree*, caddr_t key);

extern caddr_t rb_lookup_next_larger(Rb_tree*, caddr_t key);

extern caddr_t rb_lookup_next_smaller(Rb_tree*, caddr_t key);

extern caddr_t rb_lookup_smallest(Rb_tree*);

extern caddr_t rb_lookup_largest(Rb_tree*);

extern Rb_Status rb_enumerate_up(Rb_tree*, _DtCmsEnumerateProc);

extern void rb_enumerate_down(Rb_tree*, _DtCmsEnumerateProc);

extern Rb_Status rb_check_tree(Rb_tree *);

#endif
