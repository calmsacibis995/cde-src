/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $XConsortium: tt_object_list.h /main/cde1_maint/2 1995/10/07 19:09:42 pascale $ 			 				 */
/*
 *
 * tt_object_list.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */
#ifndef _TT_OBJECT_LIST_H
#define _TT_OBJECT_LIST_H
#include <stdio.h>
#include <rpc/rpc.h>
#include <util/tt_ptr.h>
#include <util/tt_object.h>
#if defined(OPT_BUG_HPUX) || defined(OPT_BUG_AIX) || defined(__osf__)
#	undef remove
#endif

#if defined(ultrix)
#include <rpc/types.h>
#endif

class _Tt_object_list_element;
class _Tt_object_list_ptr;
class _Tt_object_list: public _Tt_object {
	friend class _Tt_object_list_cursor;
      public:
        _Tt_object_list();
	_Tt_object_list(const _Tt_object_list &t);
	virtual ~_Tt_object_list();
	void flush();
	_Tt_object_list &push(const _Tt_object_ptr &e);
	_Tt_object_list &pop();
	_Tt_object_list &append(const _Tt_object_ptr &e);
	_Tt_object_list &append(_Tt_object_list_ptr l);
	_Tt_object_list &append_destructive(_Tt_object_list_ptr l);
	_Tt_object_list &dequeue();
	_Tt_object_ptr &top() const;
	_Tt_object_ptr &bot() const;
	_Tt_object_ptr &operator[] (int n) const;
	int is_empty() const;
	int count() const {
		return _count;
	}
	bool_t xdr(XDR * /* xdrs */) {
		fprintf(stderr,"Warning: _Tt_object_list::xdr called");
		return 0;
	}
	bool_t xdr(XDR *xdrs, _Tt_new_xdrfn xdrfn, _Tt_object *(*make_new)());
	void print(_Tt_object_printfn print_elt, const _Tt_ostream &os) const;
	int verify(int (*verifier)(const _Tt_object *));
      public:
        int _count;
        _Tt_object_list_element *first, *last;
};
declare_ptr_to(_Tt_object_list)

class _Tt_object_list_element : public _Tt_allocated {
	friend class _Tt_object_list;
	friend class _Tt_object_list_cursor;
      private:
	_Tt_object_list_element();
	~_Tt_object_list_element();
	_Tt_object_ptr data;
	_Tt_object_list_element *next, *prev;
};

class _Tt_object_list_cursor : public _Tt_allocated {
      public:
	_Tt_object_list_cursor();
	~_Tt_object_list_cursor();
	_Tt_object_list_cursor(const _Tt_object_list_cursor &c);
	_Tt_object_list_cursor(const _Tt_object_list_ptr &l);
	_Tt_object_ptr &operator *() const;
	_Tt_object_ptr &operator->() const;
	_Tt_object_list_cursor &reset();
	_Tt_object_list_cursor &reset(const _Tt_object_list_ptr &l);
	int next();
	int prev();
	_Tt_object_list_cursor &insert(const _Tt_object_ptr &p);
	_Tt_object_list_cursor &remove();
	int is_valid() const;
      private:
	_Tt_object_list_element *current;
	_Tt_object_list_ptr listhdr;
	enum { DELETED, INIT };
	int flags;
};

#endif				/* _TT_OBJECT_LIST_H */
