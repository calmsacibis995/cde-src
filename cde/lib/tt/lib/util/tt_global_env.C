//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $XConsortium: tt_global_env.C /main/cde1_maint/2 1995/09/06 02:33:05 lehors CHECKEDOUT $ 			 				
/*
 *
 * @(#)tt_global_env.C	1.25 95/09/26
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */
#include <unistd.h>
#include <sys/resource.h>

#include "mp/mp_mp.h"
#include "util/tt_global_env.h"
#include "util/tt_host.h"
#include "util/tt_port.h"     



_Tt_global	*_tt_global = (_Tt_global *)0;
extern          _Tt_mp * _tt_mp;

_Tt_global::
_Tt_global()
{
	silent = 0;
	_xdr_version = TT_XDR_VERSION;
	_host_cache = new _Tt_host_table(_tt_host_addr, 50);
	_host_byname_cache = new _Tt_host_table(_tt_host_name, 50);
	_multithreaded = 0;
	uid = geteuid();
	gid = getegid();
	xdisplayname = getenv("DISPLAY");
	universal_null_string = new _Tt_string_buf;
	event_counter = 0;
	next_garbage_run = -1;		// Force 1st time.
#ifdef OPT_XTHREADS
	xmutex_init(&global_mutex);
	threadkey = 0;	/* XXX = workaround for C++ bugid 4017523 */
	xthread_key_create(&threadkey, NULL);
	xcondition_init(&_proceed);
	_lock_free = 1;
#endif
}


_Tt_global::
~_Tt_global()
{
}


int _Tt_global::
find_host(_Tt_string haddr, _Tt_host_ptr &h, int create_ifnot)
{
	_Tt_host_ptr hc;


	if (_host_cache->lookup(haddr, hc)) {
		h = hc;
		return(1);
	}
	
	if (! create_ifnot) {
		return(0);
	}
	h = new _Tt_host();
        /* host not found, add it to list */
        // XXX - bugid 1138827 and 1257327, a hack to prevent a bogus
	// gethostbyaddr call from
        // happening on the client side, which results in a
        // name service packet going out on the wire to a bogus
        // IP address.  This function is designed to accept only
	// an ASCII string representation of an IP address, but
	// the server side was passing in a 4 byte binary form
	// on some occasions.  I've fixed that on the server side.

	if (! h->init_bystringaddr(haddr)) {
		return(0);
	}

	_host_cache->insert(h);
	_host_byname_cache->insert(h);
	return(1);
}


int _Tt_global::
find_host_byname(_Tt_string name, _Tt_host_ptr &h)
{
	_Tt_host_ptr hc;
	if (_host_byname_cache->lookup(name,hc)) {
		h = hc;
		return(1);
	}
	h = new _Tt_host();
	if (h->init_byname(name)) {
		_host_cache->insert(h);
		_host_byname_cache->insert(h);
		return(1);
	}
	
	return(0);
}


int _Tt_global::
get_local_host(_Tt_host_ptr &h)
{
	if (_local_host.is_null()) {
		_local_host = new _Tt_host();
		if (! _local_host->init_byname((char *)0)) {
			// Could not find IP address of local host
			// name.  The only way this has been seen to
			// happen is during system configuration
			// when a dummy name may be returned from
			// uname -n.  In this case, it seems best
			// to pretend we are localhost.

			_Tt_string localhost("localhost");

			if (! _local_host->init_byname(localhost)) {
				_local_host = (_Tt_host *)0;
				return(0);
			}
		}
	}
	
	h = _local_host;
	return(1);
}

int _Tt_global::_maxfds = -1;

int _Tt_global::
maxfds()
{
	if (_maxfds != -1) {
		return _maxfds;
	}
	_maxfds = _tt_getdtablesize();
	return _maxfds;
}

// The following version set/get functions used to be inline.  For
// some reason, cfront started generating references to the default
// new and delete operators because of this.  Of course, we can't
// tolerate that since we want our clients to link without libC.

// returns current xdr/rpc version
int _Tt_global::
xdr_version() {
	return _xdr_version;
}


// sets the current xdr version, defaults to the
// TT_XDR_VERSION constant.
void _Tt_global::
set_xdr_version(int v) {
	_xdr_version = v;
}


void _Tt_global::
grab_mutex()
{
#ifdef OPT_XTHREADS
	int lockstat;
	while (lockstat = xmutex_lock(&global_mutex)) {}

	while (!_lock_free) {
		xcondition_wait(&_proceed, &global_mutex);
	}
	_lock_free = 0;
#endif
}

void _Tt_global::
drop_mutex()
{
#ifdef OPT_XTHREADS
	int lockstat;

	_lock_free = 1;
	xcondition_signal(&_proceed);
	while (lockstat = xmutex_unlock(&global_mutex)) {}
#endif
}



