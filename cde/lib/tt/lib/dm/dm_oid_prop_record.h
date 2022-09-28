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
 * The class _Tt_oid_prop_record extends the _Tt_record type to provide the
 * behavior of an OID_PROP record.
 */

#ifndef  _TT_DM_OID_PROP_RECORD_H
#define  _TT_DM_OID_PROP_RECORD_H

#include <dm/dm_record.h>
#include <dm/dm_key.h>

struct Table_oid_prop;

class _Tt_oid_prop_record : public _Tt_record {
      public:
	_Tt_oid_prop_record();
	virtual ~_Tt_oid_prop_record();
	_Tt_key_ptr	oidkey() const;
	void		set_oidkey(_Tt_key_ptr key);
	_Tt_string	propname() const;
	void		set_propname(_Tt_string propname);
	_Tt_string	value() const;
	void		set_value(_Tt_string value);
	void		print(FILE *fs = stdout) const;
	virtual void	determine_key();
	virtual char *	bufadr();
	virtual const char *bufadr() const;
      private:
	Table_oid_prop *field;
};

declare_derived_ptr_to(_Tt_oid_prop_record,_Tt_record)

#endif  /* _TT_DM_OID_PROP_RECORD_H */
