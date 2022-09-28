/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_dbdescs_table.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * This file contains the declaration for the class _Tt_dbdescs_table which
 * implements a hash table of database descriptors.
 *
 */

#ifndef  _TT_DM_DBDESCS_TABLE_H
#define  _TT_DM_DBDESCS_TABLE_H

#include <dm/dm_db.h>
#include <dm/dm_dbref.h>
#include <util/tt_object.h>

#define  TT_DBDESCS_TABLE_BUCKETS  97

class _Tt_dbdescs_table : public _Tt_object {
      public:
	_Tt_dbdescs_table();
	virtual ~_Tt_dbdescs_table();
	_Tt_db_desc_ptr	lookup(_Tt_dbref_ptr dbref, _Tt_string db_name);
	_Tt_db_desc_ptr	insert(_Tt_dbref_ptr dbref, _Tt_string db_name);
	void		remove(_Tt_db_desc_ptr dbdesc);
	int		num_isfds();
	void		inc_num_isfds();
	void		dec_num_isfds();
      private:
	int		hash(_Tt_string envname,
			     _Tt_string path,
			     _Tt_string db_name) const;

        _Tt_db_desc_list_ptr	*bucket;
	int			_num_isfds;
};

declare_ptr_to(_Tt_dbdescs_table)

#endif  /* _TT_DM_DBDESCS_TABLE_H */
