//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $XConsortium: mp_rpc_client.C /main/cde1_maint/3 1995/10/04 13:29:35 gtsang $ 			 				
/*
 *
 * mp_rpc_auth.C
 *
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */
#include <sys/time.h>		// ultrix
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <unistd.h>
#include "tt_options.h"
#include "mp/mp_rpc_client.h"
#include "mp/mp_rpc_auth.h"
#include "mp/mp_rpc_interface.h"
#include "mp/mp_c_session.h"
#include "mp/mp_c_session_utils.h"
#include "util/tt_host.h"
#include "util/tt_port.h"


#if defined(_AIX)
/* AIX's FD_ZERO macro uses bzero() without declaring it. */
#include <strings.h>
#endif

#if defined(sgi)
/* SGI's FD_ZERO macro uses bzero() without declaring it. */
#include <CC/libc.h>
#endif	

/* 
 * Constructs an rpc auth object. 
 */
_Tt_rpc_auth::
_Tt_rpc_auth()
{
#if defined(OPT_TT_GSS_API)
        _allow_unauth_types_load = 0;
        _types_load_locked = 1;
	_compat_present = 0;
#endif // defined(OPT_TT_GSS_API)

        _auth_level = _TT_AUTH_UNIX;
        _auth_locked = 0;
	_auth_present = 0;
}


// Initialize an auth structure from the specified auth_string.
// AUTH_UNIX  is the default.
//
// return 1 if something  went wrong,
// return 0 if call succeeded.
//
int _Tt_rpc_auth::
parse_auth(_Tt_string s)
{
	_Tt_string token = s, prefix;
	struct utsname name;

	// Reset default values.
#if defined(OPT_TT_GSS_API)
        _gss_service_type = rpc_gss_svc_default;
#endif
        _auth_level = _TT_AUTH_UNIX;
        _auth_locked = 0;
	_auth_present = 1;

	_auth_init_string = s;

	if (token  == "unix") {
		_auth_level = _TT_AUTH_UNIX;
	} else

	if (token == "des") {
		_auth_level = _TT_AUTH_DES;
	} else

	if (token == "none") {
		_auth_level = _TT_AUTH_NONE;
	}
#if defined(OPT_TT_GSS_API)
	else {

		token = token.split(',', prefix);
		if  (prefix.len() == 0) {
			return 1;
		}

		if (prefix == "gss") {

			// Parse the  remaining string
			// into requests for quality of service  for:
			//	data protection (access, integrity, privacy),
			//	security mechanism,
			//	quality of data protection,
			//

			_auth_level = _TT_AUTH_GSS;


			// Parse for "protect", "mechanism", and "qop"
			// paramaters.
			//
			// Format of overall  token is
			// A,B,C, where A-C are of the
			// format property=value.
			//

			_Tt_string  property, value;
			int done, protect_set, mech_set, qop_set,  gss_lock;

			done = protect_set = mech_set = qop_set = gss_lock = 0;

			do {
				// Get next property-value token
				token = token.split(',', prefix);

				// On the  last iteration we have to do this because
				// the operation of _Tt_string::split() is a bit
				// overloaded.  If it returned the entire  remaining
				// string in prefix if the search failed, rather  than
				// in "this", we wouldn't have  to do the  reassignment.
				if (prefix.len() == 0) {
					done = 1;
					prefix = token;
				}

				value = prefix.split('=', property);
				if (property.len() == 0) {
					return 1;
				}

				if (property == "protect" && !protect_set) {
					if  (value == "access") {
						// XXX - with no FCS GSS doc I'm guessing
						// that "protect access" is the  default....
						_gss_service_type = rpc_gss_svc_default;
					} else

					if  (value == "integrity") {
						_gss_service_type = rpc_gss_svc_integrity;
					} else

					if  (value == "privacy") {
						_gss_service_type = rpc_gss_svc_privacy;
					} else {
						_gss_service_type = rpc_gss_svc_default;
					}
					protect_set = 1;
				} else

				if (property == "mechanism"  && !mech_set) {
					_gss_mechanism_name = value;
					mech_set  = 1;
				} else

				if (property == "lock"  && !gss_lock) {
					if (value == "yes") {
						_auth_locked = 1;
					} else

					if (value == "no") {
						_auth_locked = 0;
					} else {
						return 1;
					}
				} else

				if (property == "qop"   && !qop_set) {
					_gss_qop_name = value;
					qop_set  = 1;
				} else {
					return 1;
				}

			} while (!done);

		} else {
			return 1;
		}

		// Set the GSS principal name.
		if (uname(&name) == -1) {
			_tt_syslog(0, LOG_ERR, "uname(): %m");
			return 1;
		}
		_gss_principal_name = "ttsession@";
		_gss_principal_name = _gss_principal_name.cat(name.nodename);
	}
#endif

	return 0; // success!
}


#if defined(OPT_TT_GSS_API)

// Get the GSS info from the server that we cannot decode from the
// session ID string in our ennvironment, and save the values  in
// our local data structure.
//
// returns -1 on failure, 0 on success.
int
_Tt_rpc_auth::
client_gss_set_components(_Tt_c_session_ptr default_session)
{
	_Tt_gss_info_args	auth_arg;
	Tt_status		status;
 

	status = default_session->call(TT_RPC_GET_GSS_INFO,
					(xdrproc_t)xdr_void, 0,
					(xdrproc_t)tt_xdr_gss_info,
					(char *)&auth_arg);
	if (status != TT_OK) {
		return 0;
	}

	_gss_mechanism_name  = auth_arg.mechanism;
	_gss_qop_name = auth_arg.qop;
	_gss_principal_name = auth_arg.principal;
}


int _Tt_rpc_auth::
parse_compat_from_file(_Tt_string s)
{
	return _parse_compat(s, 1);
}


int _Tt_rpc_auth::
parse_compat_from_cmdline(_Tt_string s)
{
	return _parse_compat(s, 0);
}
#endif


// parse a line from the /etc/default/ttsession file.
// of the form
//	"allow_unauth_types_load=yes|no,lock=(yes|no)"
// or from the command line of the form
//	"allow_unauth_types_load=yes|no
//
// Return 0 if success
// Return 1 if failure
int _Tt_rpc_auth::
_parse_compat(_Tt_string s, int from_file)
{
	_Tt_string token = s, prefix;
	_Tt_string	property, value;
	int		done = 0;


	// Reset default values to the lest restrictive
	// options.
        _allow_unauth_types_load = 1;
        _types_load_locked = 0;
	_compat_present = 1;

	_compat_init_string = s;

	do {
		token = token.split(',', prefix);

		if (prefix.len() == 0) {
			done = 1;
			prefix = token;
		}

		value = prefix.split('=', property);
		if (property.len() == 0) {
			return 1;	// failure
		}

		if (property == "allow_unauth_types_load") {
			if (value == "yes") {
				_allow_unauth_types_load = 1;
			} else 

			if (value == "no") {
				_allow_unauth_types_load = 0;
			} else {
				return 1;	// failure
			}
		} else

		if (property == "lock" && from_file) {
			if (value == "yes") {
				_types_load_locked = 1;
			} else 

			if (value == "no") {
				_types_load_locked = 0;
			} else {
				return 1;	// failure
			}
		} else {
			return 1;
		}

	} while (!done);

	return 0; // success!
}


#if defined(OPT_TT_GSS_API)
// parse a line from the /etc/default/ttsession file.
// valid formats are:
//	COMPAT=allow_unauth_types_load=(yes|no),lock=(yes|no)
//	AUTH=gss,.... (same as the ttsession command line option)
//
// Return 0 if success
// Return 1 if failure
//
int _Tt_rpc_auth::
parse_file_line(_Tt_string s)
{
	_Tt_string prefix, token=s;


	// Get next property-value token
	// Valid values are currently:
	//	AUTH=value
	//	COMPAT=value
	token = token.split('=', prefix);

	if (prefix == "AUTH") {
		return(parse_auth(token));
	} else

	if (prefix == "COMPAT") {
		return(parse_compat_from_file(token));
	}

	return 1;	// failure
}
#endif


void _Tt_rpc_auth::
print()
{
	fprintf(stderr, "_Tt_rpc_auth:: object is:\n");

	fprintf(stderr, "\t_Tt_auth_level = %d\n", (int) _auth_level);

#if defined(OPT_TT_GSS_API)
	fprintf(stderr, "\trpc_gss_service_t = %d\n", _gss_service_type);

	fprintf(stderr, "\t_gss_mechanism_name = %d\n", 
	    _gss_mechanism_name.is_null() ? "(nil)" : (char *)_gss_mechanism_name);

	fprintf(stderr, "\t_gss_qop_name = %d\n", 
		_gss_qop_name.is_null() ? "(nil)" : (char *)_gss_qop_name);

	fprintf(stderr, "\t_gss_principal_name = %d\n", 
		_gss_principal_name.is_null() ? "(nil)" : (char *)_gss_principal_name);
#endif

	fprintf(stderr, "\t_auth_init_string = %d\n", 
		_auth_init_string.is_null() ? "(nil)" : (char *)_auth_init_string);

	fprintf(stderr, "\t_auth_locked = %d\n", _auth_locked);

	fprintf(stderr, "\t_allow_unauth_types_load = %d\n", _allow_unauth_types_load);

	fprintf(stderr, "\t_types_load_locked = %d\n", _types_load_locked);
	fprintf(stderr, "(End)\n");
}
