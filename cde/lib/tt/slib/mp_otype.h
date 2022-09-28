/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *  Tool Talk Message Passer (MP) - mp_otype.h
 *
 *  Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 *  Declaration of the _Tt_otype class representing an in-memory object type
 *  definition.
 */

#ifndef _MP_OTYPE_H
#define _MP_OTYPE_H

#include "mp_signature.h"
#include "util/tt_table.h"

typedef char **(*_Tt_otype_filter)(char *otid,
					   void *context,
					   void *accumulator);
class _Tt_types_table;

class _Tt_otype : public _Tt_object {
      public:
	_Tt_otype();
	_Tt_otype(_Tt_string otid);
	virtual ~_Tt_otype();
	_Tt_string_list_ptr	&children();
	_Tt_string_list_ptr	&parents();
	const _Tt_string &	otid() const {return _otid;};
	void			set_otid(_Tt_string s);
	_Tt_signature_list_ptr	&hsigs();
	_Tt_signature_list_ptr	&osigs();
	void			set_ancestors(_Tt_string_list_ptr ancs);
	void			append_inhs(_Tt_signature_ptr sig);
	void			append_osig(_Tt_signature_ptr sig);
	void			append_hsig(_Tt_signature_ptr sig,
					Tt_category category);
	void			append_osigs(_Tt_signature_list_ptr sigs);
	void			append_hsigs(_Tt_signature_list_ptr sigs,
					Tt_category category);
	void			print(const _Tt_ostream &os) const;
	void			pretty_print(const _Tt_ostream &os) const;
	bool_t			xdr(XDR *xdrs);
	void			*ce_entry;
	int			xdr_version_required() const;
	friend	class	_Tt_types_table;
      private:
	friend class _Tt_typedb;
	_Tt_string		_otid;
	_Tt_string_list_ptr	_ancestors;
	_Tt_string_list_ptr	_children;
	_Tt_signature_list_ptr	_inhs;
	_Tt_signature_list_ptr	_osigs;
	_Tt_signature_list_ptr	_hsigs;
	int			visitp;
};
_Tt_string _tt_otype_otid(_Tt_object_ptr &o);
void _tt_ptype_print(const _Tt_ostream &os, const _Tt_object *obj);

#endif				/* _MP_OTYPE_H */
