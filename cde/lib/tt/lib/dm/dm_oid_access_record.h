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
 * The class _Tt_oid_access_record extends the _Tt_record type to provide the
 * behavior of an OID_ACCESS record.
 */

#ifndef  _TT_DM_OID_ACCESS_RECORD_H
#define  _TT_DM_OID_ACCESS_RECORD_H

#include <dm/dm_record.h>
#include <dm/dm_key.h>

struct Table_oid_access;

class _Tt_oid_access_record : public _Tt_record {
      public:
	_Tt_oid_access_record();
	virtual ~_Tt_oid_access_record();
	_Tt_key_ptr	oidkey() const;
	void		set_oidkey(_Tt_key_ptr key);
	uid_t		owner() const;
	void		set_owner(uid_t owner);
	gid_t		group() const;
	void		set_group(gid_t group);
	mode_t		mode() const;
	void		set_mode(mode_t mode);
	void		print(FILE *fs = stdout) const;
	virtual void	determine_key();
	virtual char *	bufadr();
	virtual const char *bufadr() const;
      private:
	Table_oid_access *field;
};

declare_derived_ptr_to(_Tt_oid_access_record,_Tt_record)

#endif  /* _TT_DM_OID_ACCESS_RECORD_H */
