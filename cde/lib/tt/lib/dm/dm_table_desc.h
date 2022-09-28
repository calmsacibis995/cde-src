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
 *  The class _Tt_table_desc implements a type that contains information about
 *  a database's location and its table layout.  A pointer to an object of this
 *  type is used as a database reference.
 */

#ifndef  _TT_DM_TABLE_DESC_H
#define  _TT_DM_TABLE_DESC_H

#include <dm/dm_enums.h>
#include <dm/dm_key_desc.h>

declare_ptr_to(_Tt_table_desc)

class _Tt_table_desc : public _Tt_object {
      public:
	enum Recfmt {RECFMT_F,RECFMT_V};
	_Tt_table_desc();
	~_Tt_table_desc();
	_Tt_table_desc(_Tt_dbtable_id id, Recfmt r, short min_len);
	_Tt_dbtable_id		tableid() const;
	int	        	variable_recp() const;
	short           	minreclen() const;
	_Tt_key_desc_list_ptr	keys();
	_Tt_key_desc_ptr	findkeydesc(_Tt_key_id key_id);
	int	        	primarykeyp() const;
	void			appendkey(_Tt_key_desc_ptr key);
      private:
        _Tt_dbtable_id		id;	/* name of table */
	Recfmt			recfmt;
	short           	min_rec_len; /* minimum record length */
	_Tt_key_desc_list_ptr	_keys;
};


#endif /* _TT_DM_TABLE_DESC_H */
