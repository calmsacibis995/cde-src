/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_redirect.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Declaration of class _Tt_redirect which implements a <host1 -> host2:path>
 * redirection entry.
 *
 */

#ifndef  _TT_DM_REDIRECT_H
#define  _TT_DM_REDIRECT_H

#include <util/tt_string.h>
#include <util/tt_table.h>

class _Tt_redirect : public _Tt_object {
      public:
	_Tt_redirect();
	~_Tt_redirect();
	_Tt_redirect(const char *h1, const char *h2, const char *p2);
	_Tt_string	&host1() { return _host1; }
	void		set_host1(_Tt_string h1) { _host1 = h1; }
	_Tt_string	host2() { return _host2; }
	void		set_host2(_Tt_string h2) { _host2 = h2; }
	_Tt_string	path2() { return _path2; }
	void		set_path2(_Tt_string p2) { _path2 = p2; }
      private:
	_Tt_string	_host1;
	_Tt_string	_host2;
	_Tt_string	_path2;
};

declare_list_of(_Tt_redirect)

declare_table_of(_Tt_redirect)
_Tt_string _tt_redirect_host1(_Tt_object_ptr &o);
#endif  /* _TT_DM_REDIRECT_H */
