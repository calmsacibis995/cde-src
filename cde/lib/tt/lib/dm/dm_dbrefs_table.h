/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_dbrefs_table.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * This file contains the declaration for the class _Tt_dbrefs_table which
 * implements a hash table of database references.
 *
 */

#ifndef  _TT_DM_DBREFS_TABLE_H
#define  _TT_DM_DBREFS_TABLE_H

#include <dm/dm_dbref.h>

#define  TT_DBREFS_TABLE_BUCKETS  97

class _Tt_dbrefs_table : public _Tt_object {
      public:
	_Tt_dbrefs_table();
	virtual ~_Tt_dbrefs_table();
	_Tt_dbref_ptr	lookup(_Tt_name_space ns,
			       const char *envname, const char *relpath);
	_Tt_dbref_ptr	insert(_Tt_string ns, _Tt_string env, _Tt_string path);
	_Tt_dm_status	insert(_Tt_string dbrefstr, _Tt_dbref_ptr &dbref);
      private:
	int		hash(const char *envname, const char *relpath) const;
        _Tt_dbref_list_ptr	*bucket;
};

declare_ptr_to(_Tt_dbrefs_table)

#endif  /* _TT_DM_DBREFS_TABLE_H */
