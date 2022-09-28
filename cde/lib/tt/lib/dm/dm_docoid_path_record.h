/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_docoid_path_record.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * The class _Tt_docoid_path_record extends the _Tt_record type to provide the
 * behavior of an DOCOID_PATH record.
 */

#ifndef  _TT_DM_DOCOID_PATH_RECORD_H
#define  _TT_DM_DOCOID_PATH_RECORD_H

#include <dm/dm_record.h>
#include <dm/dm_key.h>

struct Table_docoid_path;

class _Tt_docoid_path_record : public _Tt_record {
      public:
	_Tt_docoid_path_record();
	virtual ~_Tt_docoid_path_record();
	_Tt_key_ptr	docoid_key() const;
	void		set_docoid_key(_Tt_key_ptr oidkey);
	_Tt_string	path() const;
	void		set_path(_Tt_string path);
	void		print(FILE *fs = stdout) const;
	virtual void	determine_key();
	virtual char *	bufadr();
	virtual const char *bufadr() const;
      private:
	Table_docoid_path *field;
};

declare_derived_ptr_to(_Tt_docoid_path_record,_Tt_record)

#endif  /* _TT_DM_DOCOID_PATH_RECORD_H */
