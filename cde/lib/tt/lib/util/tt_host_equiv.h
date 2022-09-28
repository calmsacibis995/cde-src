/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * tt_host_equiv.h
 *
 * Copyright (c) 1994 by Sun Microsystems, Inc.
 *
 * This file implements the _Tt_host_equiv object which is a utility object
 * that handles determining if one hostname refers to the same host
 * as that of another hostname.
 */
#ifndef _TT_HOST_EQUIV_H
#define _TT_HOST_EQUIV_H

#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "util/tt_string.h"
#include "util/tt_table.h"

declare_list_of(_Tt_hostname_cache)
declare_table_of(_Tt_hostname_cache)
declare_ptr_to(_Tt_host_equiv)

class _Tt_hostname_cache : public _Tt_object {
	public:
		// a null constructor is required by the
		// table_of macro; DO NOT USE THIS.
		_Tt_hostname_cache();

		_Tt_hostname_cache(_Tt_string & hostname);

		~_Tt_hostname_cache();

		// key functions must be static
		static _Tt_string h_keyfn(_Tt_object_ptr & p);

		_Tt_string hostname_val() { return hostname; }

		_Tt_string	hostname;

		// length of each address in _addr_list
		int addr_length;

		_Tt_string_list_ptr addr_list;
};

class _Tt_host_equiv : public _Tt_object {
      public:
		_Tt_host_equiv();

		~_Tt_host_equiv();

		_Tt_string prefix_host(const _Tt_string & host1,
				       const _Tt_string & localhost);

		int hostname_equiv(const _Tt_string & host1,
				   const _Tt_string & host2);
      private:
		_Tt_hostname_cache_table_ptr _cache_table;
};

#endif				/* _TT_HOST_EQUIV_H */
