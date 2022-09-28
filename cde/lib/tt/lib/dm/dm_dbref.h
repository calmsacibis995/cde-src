/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_dbref.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Declaration of class _Tt_dbref which implements a reference object to a LS
 * database.  The reference consists of the name of the host machine and the
 * TT_DB directory path where the LS databases are located.  The TT_DB
 * directory is right under the partition mount point where the associated
 * application documents are located.  The flat layout of a dbref is:
 *
 *	<envname'\0'> <relpath'\0'>
 */

#ifndef  _TT_DM_DBREF_H
#define  _TT_DM_DBREF_H

#include <dm/dm_enums.h>
#include <util/tt_string.h>
#include <util/tt_list.h>

class _Tt_db_desc;
class _Tt_dbdescs_table;
class _Tt_dbrefs_table;
class _Tt_link_ends_record;
class _Tt_oid;

declare_list_of(_Tt_dbref)

class _Tt_dbref : public _Tt_object {
      public:
	_Tt_dbref();
	_Tt_dbref(_Tt_name_space ns, const char *path, const char *env = 0);
	virtual ~_Tt_dbref();
	_Tt_string	db_path();
	operator	_Tt_string();
	void		print(FILE *fs = stdout) const;
	int operator==	(const _Tt_dbref_ptr dbref) const;
	friend 	_Tt_db_desc;
	friend 	_Tt_dbdescs_table;
	friend	_Tt_dbrefs_table;
	friend	_Tt_link_ends_record;
	friend	_Tt_oid;
      private:
	_Tt_name_space	namespace;
	_Tt_string	envname;  /* root env if NSE, host name otherwise */
	_Tt_string	relpath;  /* path to db directory relative to host */
};

extern const char *namespace_str[];	/* namespace strings array */

#endif  /* _TT_DM_DBREF_H */
