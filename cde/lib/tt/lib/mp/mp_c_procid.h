/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $XConsortium: mp_c_procid.h /main/cde1_maint/1 1995/07/14 17:55:17 drk CHECKEDOUT $ 			 				 */
/* @(#)mp_c_procid.h	1.11 93/07/30
 * 
 * Copyright (c) 1990, 1992 by Sun Microsystems, Inc.
 * 
 * This file implements the client side of the _Tt_procid object.
 */
#ifndef _MP_C_PROCID_H
#define _MP_C_PROCID_H
#include <mp/mp_procid.h>
#include <mp/mp_c_session.h>
#include <mp/mp_c_message_utils.h>
#include "api/c/api_typecb_utils.h"

class _Tt_c_procid : public _Tt_procid {
      public:
	_Tt_c_procid();
	~_Tt_c_procid();
	_Tt_c_procid(const _Tt_string &id);
	Tt_status		add_pattern(_Tt_pattern_ptr &p);
	Tt_status		api_in(_Tt_string &s);
	_Tt_string		api_out();
	void			clear_signal();
	void			close();
	Tt_status		commit();
	Tt_status		declare_ptype(_Tt_string &ptid);
	Tt_status		undeclare_ptype(_Tt_string &ptid);
	Tt_status		exists_ptype(_Tt_string &ptid);
	Tt_status		unblock_ptype(const _Tt_string &ptid);
	Tt_status		load_types(_Tt_string &typebuffer);
	_Tt_string 		&default_file() {
		return _default_file;
	}
	_Tt_string		&default_ptype() {
		return _default_ptype;
	}
	_Tt_c_session_ptr		&default_session();

	Tt_status		del_pattern(const _Tt_string &id);
	Tt_status		init();
	Tt_status		next_message(_Tt_c_message_ptr &m);
	void			set_default_ptype(_Tt_string &ptid);
	void			set_default_file(const _Tt_string &file);
	Tt_status		set_default_session(_Tt_string &id);
	int			set_fd_channel(int port = 0);
	Tt_status		update_message(const _Tt_c_message_ptr &msg,
					       Tt_state newstate);
	_Tt_typecb_table_ptr &	ptype_callbacks() {
		return _ptype_callbacks;
	}
	_Tt_typecb_table_ptr &	otype_callbacks() {
		return _otype_callbacks;
	}
      private:
	_Tt_string		_default_file; 
	_Tt_string		_default_ptype;
	_Tt_c_session_ptr	_default_session;
	_Tt_message_list_ptr	_undelivered;
	_Tt_c_message_ptr	_unvoted;
	_Tt_typecb_table_ptr 	_ptype_callbacks;
	_Tt_typecb_table_ptr 	_otype_callbacks;
};

#endif				/* _MP_C_PROCID_H */
