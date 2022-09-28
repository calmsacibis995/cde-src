/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* 
 * mp_c_session.h
 * 
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 * 
 * This file contains the implementation of the client side of
 * the _Tt_session object.
 */
#ifndef MP_C_SESSION_H
#define MP_C_SESSION_H
#include <mp/mp_session.h>
#include <mp/mp_c_session_utils.h>

class _Tt_c_session : public _Tt_session {
      public:
	_Tt_c_session();
	virtual ~_Tt_c_session();

	Tt_status		c_addprop(_Tt_string prop, _Tt_string val);
	Tt_status		c_getprop(_Tt_string prop, int i,
					  _Tt_string &value);
	Tt_status		c_init();
	Tt_status		c_join(_Tt_procid_ptr &procid);
	Tt_status		c_propcount(_Tt_string prop, int &cnt);
	Tt_status		c_propname(int i, _Tt_string &prop);
	Tt_status		c_propnames_count(int &cnt);
	Tt_status		c_quit(_Tt_procid_ptr &procid);
	Tt_status		c_setprop(_Tt_string prop, _Tt_string val);
      private:
	int			startup_ttsession(_Tt_string &command);
};

#endif				/*  MP_C_SESSION_H */
