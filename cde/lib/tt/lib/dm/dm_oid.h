/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_oid.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * This file contains the class declaration for a generic oid.  The flat string
 * layout of an oid is:
 *
 *	<key> <dbref>
 *
 */

#ifndef  _TT_DM_OID_H
#define  _TT_DM_OID_H

#include <dm/dm_oid_utils.h>
#include <dm/dm_dbref.h>
#include <dm/dm_key.h>
#include <util/tt_object.h>
#include <util/tt_string.h>

class _Tt_oid : public _Tt_object {
      public:
	_Tt_oid();
	_Tt_oid(_Tt_dbref_ptr dbref);
	_Tt_oid(_Tt_dbref_ptr dbref, _Tt_key_ptr key);
	virtual ~_Tt_oid();
	_Tt_dm_status	open(_Tt_string nodeid);
	_Tt_dm_status	create(_Tt_string path);
	_Tt_dm_status	create(_Tt_string path, _Tt_key_ptr key);
	_Tt_key_ptr	key() const;
	_Tt_dbref_ptr	dbref();
	_Tt_string	otype();
	operator	_Tt_string();
	int operator==	(const _Tt_oid_ptr nodeid) const;
	_Tt_dm_status	error_code() { return errcode; }
	_Tt_string	error_string();
	void		print(FILE *fs = stdout) const;
      private:
	static	_Tt_dm_status	errcode;
	_Tt_dbref_ptr		_dbref;
	_Tt_key_ptr		_key;
};
#endif /* _TT_DM_OID_H */
