/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* -*-C++-*-
 *
 * @(#)api_typecb.h	1.2 93/07/30 SMI
 *
 * Copyright (c) 1993 by Sun Microsystems, Inc.
 * 
 * _Tt_typecb is a class which holds a <typename, opnum, callback list>
 * triple.  Tables of these are attached to _Tt_c_procid instances to hold
 * the callbacks registerd by tt_ptype_opnum_callback_add and
 * tt_otype_opnum_callback_add.
 */

#ifndef _API_TYPECB_H
#define _API_TYPECB_H

#include "util/tt_object.h"
#include "api/c/tt_c.h"
#include "api/c/api_handle.h"


class _Tt_typecb : public _Tt_object {
      public:
	_Tt_typecb();
	_Tt_typecb(const _Tt_string &typename, int opnum);
	virtual ~_Tt_typecb();
	_Tt_string &	typename();
	int		opnum();
	void		add_callback(Tt_message_callback f);
	Tt_callback_action run_callbacks(Tt_message mh, Tt_pattern ph);
	void	       *clientdata;
      private:
	_Tt_string	_typename;
	int		_opnum;
	_Tt_api_callback_list_ptr cblp;
};
	


#endif				/* _MP_PATTERN_H */
