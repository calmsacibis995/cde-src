/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* -*-C++-*- */
/*
 * @(#)tt_ptr.h	1.22 94/11/17
 * tt_ptr.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 */

#if !defined(_TT_PTR_H)
#define _TT_PTR_H

#include "tt_options.h"
#if defined(OPT_BUG_SUNOS_4) && defined(__GNUG__)
#	include <stdlib.h>
#endif
#include <rpc/rpc.h>
#include "util/tt_generic.h"
//
// template for xdr function callbacks required by _Tt_new_ptr::xdr.
// These will typically be generated by the implement_ptr_to macro in
// tt_ptr.h
//
class _Tt_object;
typedef bool_t (*_Tt_new_xdrfn)(XDR *xdrs, _Tt_object *obj);
#include <util/tt_new.h>
#include <util/tt_new_ptr.h>

/* This is for HP, whose stdio.h redefines _ptr */
#ifdef _ptr
#undef _ptr
#endif

#define ptr_to(t) name2(t,_ptr)

#define constructor_of(t) name2(t,_maker)
#define xdrfn_of(t) name2(t,_xdr)
#define declare_ptr_to(TYPE)						     \
class TYPE;								     \
TYPE *constructor_of(TYPE)();						     \
class ptr_to(TYPE): public _Tt_new_ptr {				     \
      public:								     \
	ptr_to(TYPE) (TYPE *s) : _Tt_new_ptr((_Tt_object *)s) {};     	     \
	ptr_to(TYPE) ();						     \
	ptr_to(TYPE) (const ptr_to(TYPE) &x) : _Tt_new_ptr(x) {};	     \
	~ptr_to(TYPE) () ;					             \
	int 	       is_null() const { return((TYPE *)sb == (TYPE *)0); }  \
	int	       is_eq(const ptr_to(TYPE) &x) const { return(sb==x.sb);}\
	ptr_to(TYPE) & operator=(const ptr_to(TYPE) &x) {		     \
		return (ptr_to(TYPE)&)_Tt_new_ptr::operator=(x);	     \
	}								     \
	ptr_to(TYPE) & operator=(TYPE *x) {				     \
	   return (ptr_to(TYPE)&)_Tt_new_ptr::operator=((_Tt_object*)x);     \
        }								     \
	TYPE &	       operator *() const {return *((TYPE *)sb);}	     \
	TYPE *	       operator ->() const {return (TYPE *)sb;}		     \
	TYPE *         c_pointer() const {return (TYPE *)sb;}		     \
	bool_t	       xdr(XDR *xdrs);					     \
};

#define declare_derived_ptr_to(TYPE,PTYPE)				     \
class TYPE;								     \
TYPE *constructor_of(TYPE)();						     \
class ptr_to(TYPE) : public ptr_to(PTYPE) {				     \
      public:								     \
	ptr_to(TYPE) (TYPE *s) : ptr_to(PTYPE)((PTYPE *)s) {}		     \
	ptr_to(TYPE) ();						     \
	ptr_to(TYPE) (const ptr_to(TYPE) &x) : ptr_to(PTYPE)(x) {}	     \
	~ptr_to(TYPE) (); 					     \
	int 	       is_null() const { return(sb == 0); }		     \
	int	       is_eq(const ptr_to(TYPE) &x) const		     \
			{ return((TYPE *)sb==(TYPE *)x.sb);}		     \
	ptr_to(TYPE) & operator=(const ptr_to(TYPE) &x) {		     \
               return (ptr_to(TYPE)&)_Tt_new_ptr::operator=(x);		     \
        }								     \
	ptr_to(TYPE) & operator=( TYPE *x) {				     \
		return (ptr_to(TYPE)&)_Tt_new_ptr::operator=((_Tt_object *)x); \
	}								     \
	TYPE &	       operator *() const {return *(TYPE *)sb;}		     \
	TYPE *	       operator ->() const {return (TYPE *)sb;}		     \
	TYPE *         c_pointer() const {return (TYPE *)sb;}		     \
	bool_t	       xdr(XDR *xdrs);					     \
};

#define implement_ptr_to(TYPE)						     \
ptr_to(TYPE)::								     \
~ptr_to(TYPE)()								     \
{									     \
}									     \
									     \
ptr_to(TYPE)::								     \
ptr_to(TYPE)()								     \
{									     \
}									     \
                                                                             \
TYPE *									     \
constructor_of(TYPE)()							     \
{									     \
	return new TYPE;						     \
}									     \
									     \
bool_t									     \
xdrfn_of(TYPE)(XDR *xdrs, _Tt_object *p)				     \
{									     \
	return ((TYPE *)p)->xdr(xdrs);					     \
}									     \
									     \
bool_t ptr_to(TYPE)::							     \
xdr(XDR *xdrs)								     \
{									     \
	return _Tt_new_ptr::xdr(xdrs,					     \
				(_Tt_new_xdrfn)xdrfn_of(TYPE),		     \
				(_Tt_object *(*)())constructor_of(TYPE));	     \
}							


#define implement_derived_ptr_to(TYPE,PTYPE)   implement_ptr_to(TYPE)

#endif


