/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_key.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 *  The class _Tt_key implements a unique OID key.  The key layout is:
 *
 *	<version number:2>  <zero pad:2> <host id:4> <time sec:4> <time usec:4>
 *  The pad goes before the host id to align the host id on an int boundary.
 */

#ifndef  _TT_DM_KEY_H
#define  _TT_DM_KEY_H

#include <tt_const.h>
#include <util/tt_object.h>
#include <util/tt_string.h>

declare_ptr_to(_Tt_key)

class _Tt_key : public _Tt_object {
      public:
	_Tt_key(short ver_num = 0);
	_Tt_key(_Tt_key_ptr key);
	_Tt_key(_Tt_string s);
	_Tt_key(const unsigned char *s);
	virtual ~_Tt_key();
	operator	_Tt_string() const;
	short		version_number() const;
	const char	*content() const;     /* whole key including version */
	int operator==	(const _Tt_key &key) const;
	int operator!=	(const _Tt_key &key) const
		{ return ! operator==(key);}
	void		print(FILE *fs = stdout) const;
      private:
	struct keystruct {
		unsigned short version;
		unsigned short padding;
		unsigned long hostid;
		unsigned long time_sec;
		unsigned long time_usec;
	} _key;
};

#endif  /* _TT_DM_KEY_H */
