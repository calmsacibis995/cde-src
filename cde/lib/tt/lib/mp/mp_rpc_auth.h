/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * mp_rpc_auth.h
 *
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 *
 * Declaration of
 * _Tt_rpc_auth which represents the ttsession auth object.
 */
#ifndef _TT_MP_RPC_AUTH
#define _TT_MP_RPC_AUTH

#include "tt_options.h"
#include "util/tt_object.h"
#include "util/tt_string.h"
#include "mp/mp_rpc_auth_utils.h"
#include "mp/mp_c_session_utils.h"
#include "mp/mp_xdr_functions.h"

#define	_AUTH_DEFAULT_CONFIG	"/etc/default/ttsession"

#define	CMDLINE_ALLOW_TYPES_LOAD	0x01
#define	CMDLINE_ALLOW_GSS		0x02
#define	FILE_ALLOW_TYPES_LOAD		0x04
#define	FILE_ALLOW_GSS			0x08

/*
 * As of the CDE release of ToolTalk, _TT_AUTH_XAUTH is no longer supported.
 * It has been removed along with all references in the TT code, but the
 * value of _TT_AUTH_DES was not changed to the now-missing value of 1.
 */
typedef enum {
        _TT_AUTH_UNIX   = 0,    /* Unix "authentication" (default) */
        _TT_AUTH_DES    = 2,    /* Secure RPC (DES encryption)     */
        _TT_AUTH_NONE   = 3,    /* No authentication */
#if defined(OPT_TT_GSS_API)
        _TT_AUTH_GSS    = 4     /* GSS RPC Security Interface */
#endif
}_Tt_auth_level;

class _Tt_rpc_auth : public _Tt_object {
	public:
		_Tt_rpc_auth();
		~_Tt_rpc_auth()
		{
			// cl_auth security context in the  case  of GSS is
			// destroyed in the same class it's created,
			// _Tt_c_procid::. For AUTH_UNIX and AUTH_DES they
			// are also destroyed in the same place they are
			// created, but it's in _Tt_rpc_client::
		};

		// For user in the server  to init auth string
		int parse_auth(_Tt_string s);

#if defined(OPT_TT_GSS_API)
		// Since the syntax for the compat arts  are
		// not the same on the command line as from
		// the /etc/default/ttsession file, we have this.
		int parse_compat_from_file(_Tt_string s);
		int parse_compat_from_cmdline(_Tt_string s);

		// Parse lines  from the _AUTH_DEFAULT_CONFIG file
		int parse_file_line(_Tt_string s);
#endif

		// This is  intended  for use in the client, which
		// sets the auth level in _Tt_session::parsed_address()
		//
		void client_set_auth_level(_Tt_auth_level val) {
			_auth_level  = val;
		};

		_Tt_auth_level	auth_level() {
			return _auth_level;
		};

#if defined(OPT_TT_GSS_API)
		// Called in the client to do a tt_xdr_gss_info()
		// RPC to get the  servers GSS configuration values,
		// and set the clients copy of them.
		int client_gss_set_components(_Tt_c_session_ptr default_session);

	
		int	auth_present() {
			return _auth_present;
		};
	
		int	auth_locked() {
			return _auth_locked;
		};
	
		rpc_gss_service_t gss_service() {
			return _gss_service_type;
		};
	
		_Tt_string & gss_mechanism() {
			return _gss_mechanism_name;
		};
	
		_Tt_string & gss_qop() {
			return _gss_qop_name;
		};

		_Tt_string & gss_principal() {
			return _gss_principal_name;
		};

		int allow_types_load() {
			return _allow_unauth_types_load;
		};

		int compat_present() {
			return _compat_present;
		};

		int types_load_locked() {
			return _types_load_locked;
		};
#endif

		void print();

		_Tt_string auth_init_string() {
			return _auth_init_string;
		};

		_Tt_string compat_init_string() {
			return _compat_init_string;
		};


	private:
		_Tt_auth_level		_auth_level;

		// AUTH arg, saved for posterity.
		_Tt_string	_auth_init_string;

#if defined(OPT_TT_GSS_API)
		// GSS-specific auth data members.
		rpc_gss_service_t	_gss_service_type;
		_Tt_string		_gss_mechanism_name;
		_Tt_string		_gss_qop_name;
		_Tt_string		_gss_principal_name;

		// is the auth specification locked?
		int		_auth_locked;

		// COMPATH arg, saved for posterity.
		_Tt_string	_compat_init_string;

                int		_auth_present;
                int		_compat_present;

		// For specifying that tt_session_types_load() should
		// be allowed or not...
		int			_allow_unauth_types_load;
		int			_types_load_locked;

		int			_parse_compat(_Tt_string s,
						      int from_file);
#endif
};

#endif				/* _TT_MP_RPC_AUTH */
