/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Message Passer (MP) - mp_signature.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Declaration of the _Tt_signature & _Tt_signature_list classes.
 */

#ifndef _MP_SIGNATURE_H
#define _MP_SIGNATURE_H
#include "mp/mp_global.h"
#include "mp/mp_arg_utils.h"
#include "mp_signature_utils.h"
#include "mp/mp_context_utils.h"
#include "mp/mp_trace.h"
/*
 * _Tt_signature is used to represent the signature of a method or
 * procedure. 
 */



class _Tt_signature : public _Tt_object {
      public:
	_Tt_signature();
	_Tt_signature(_Tt_signature_ptr sig);
	virtual ~_Tt_signature();
	Tt_scope		scope() const;
	const _Tt_arg_list_ptr	&args() const;
	const _Tt_context_list_ptr	&contexts() const;
	Tt_class		message_class() const;
	Tt_category		category() const;
	const _Tt_string &	op() const;
	int			opnum() const;
	Tt_disposition		reliability() const;
	const _Tt_string &	ptid() const;

	/* only relevant for otype signatures */
	const _Tt_string &	otid() const;
	_Tt_string		super_otid();

	void			set_scope(Tt_scope scope);
	void			set_args(_Tt_arg_list_ptr &args);
	void			set_message_class(Tt_class mclass);
	void			set_pattern_category(Tt_category kind);
	void			set_op(_Tt_string op);
	void			set_opnum(int opnum);
	void			set_reliability(Tt_disposition r);
	void			set_ptid(_Tt_string ptid);
	void			set_otid(_Tt_string otid);
	void			set_super_otid(_Tt_string otid);
	int			match(
					Tt_scope s,
					Tt_class c,
					const _Tt_string &op,
					const _Tt_arg_list_ptr &args,
					const _Tt_string &otype,
					const _Tt_context_list_ptr &contexts,
					const _Tt_msg_trace *trace,
					int &was_exact
				) const;
	int			match(
					const _Tt_string &op,
					const _Tt_arg_list_ptr &args,
					const _Tt_context_list_ptr &contexts,
					_Tt_string &reason,
					int &was_exact
				) const;
	void			append_arg(_Tt_arg_ptr &arg);
	void			append_context(_Tt_context_ptr &c);
	void			print(const _Tt_ostream &os) const;
	void			pretty_print(const _Tt_ostream &os) const;
	static void		pretty_print_(const _Tt_ostream &os,
					const _Tt_object *obj);
	static void		pretty_print_(const _Tt_ostream &os,
					const _Tt_signature_list_ptr &sigs,
					Tt_category category,
					int skip_sigs_with_otid);
	bool_t			xdr(XDR *xdrs);
	int			operator==(const _Tt_signature &sig) const;
	int			is_same_method(const _Tt_signature &sig) const;
	int			is_ce_equal(_Tt_signature &st);
	void			*ce_entry;
	unsigned int		timestamp() const
					{ return _timestamp; }
	void			set_timestamp(unsigned int stamp)
					{ _timestamp = stamp; }
	int			xdr_version_required() const;
	static int		xdr_version_required_(
					const _Tt_signature_list_ptr &sigs);
      private:
	friend class _Tt_typedb;
	_Tt_string		_mangled_args;
	Tt_scope		_scope;
	_Tt_arg_list_ptr	_args;
	_Tt_context_list_ptr	_contexts;
	Tt_class		_message_class;
	Tt_category		_pattern_category;
	_Tt_string		_op;
	Tt_disposition		_reliability;
	int			_opnum;
	_Tt_string		_ptid;
	_Tt_string		_otid;
	_Tt_string		_super_otid;
	unsigned int		_timestamp;
};


#endif				/* _MP_SIGNATURE_H */
