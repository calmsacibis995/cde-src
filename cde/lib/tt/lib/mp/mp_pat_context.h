/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* -*-C++-*-
 *
 * @(#)mp_pat_context.h	1.4 30 Jul 1993
 *
 * Tool Talk Message Passer (MP) - mp_pat_context.h
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 *
 * _Tt_pat_context is a context slot in a pattern.
 */

#ifndef _MP_PAT_CONTEXT_H
#define _MP_PAT_CONTEXT_H

#include <mp/mp_context.h>
#include <mp/mp_pat_context_utils.h>
#include <mp/mp_msg_context.h>
#include <mp/mp_arg_utils.h>
#include <rpc/rpc.h>

class _Tt_pat_context : public _Tt_context {
      public:
	_Tt_pat_context();
	_Tt_pat_context(const _Tt_context &);
	virtual ~_Tt_pat_context();

	Tt_status		addValue(
					const _Tt_string &value
				);
	Tt_status		addValue(
					int value
				);
	Tt_status		addValue(
					const _Tt_msg_context &msgCntxt
				);
	Tt_status		deleteValue(
					const _Tt_msg_context &msgCntxt
				);

	void			print(
					const _Tt_ostream &os
				) const;
	static void		print_(
					const _Tt_ostream &os,
					const _Tt_object *obj
				);
	bool_t			xdr(
					XDR *xdrs
				);

      protected:
	_Tt_arg_list_ptr	_values;
};

#endif				/* _MP_PAT_CONTEXT_H */
