/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM)
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 *  The class _Tt_key_desc implements a type that contains information about
 *  a NetISAM record's key layout.
 */

#ifndef  _TT_DM_KEY_DESC_H
#define  _TT_DM_KEY_DESC_H

#include <dm/dm_enums.h>
#include <util/tt_object.h>
#include <util/tt_list.h>

struct keydesc;			/* fully defined in isam.h */


class _Tt_key_desc : public _Tt_object {     /* description of a key's fields */
      public:
	_Tt_key_desc() {}
	_Tt_key_desc(_Tt_key_id key_id);
	virtual ~_Tt_key_desc();
        _Tt_key_id      keyid() const;
	keydesc        *key_desc();
	const keydesc  *key_desc() const;
	int	        dupsp() const;
	void            setdups();
	void		setnodups();
	short		num_parts() const;
	void		setnumparts(short nparts);
	void		setkeypart(short part_no, short kp_start,
				   short kp_len, short kp_type);
	short		part_start(short part_no = 0) const;
	short		part_len(short part_no = 0) const;
	short		part_type(short part_no = 0) const;
	short		maxlength() const;
	void		print(FILE *fs = stdout) const;
      private:
        _Tt_key_id      id;
        keydesc         *kd; /* key descriptor in NetISAM format */
};

declare_list_of(_Tt_key_desc)

#endif  /* _TT_DM_KEY_DESC_H */
