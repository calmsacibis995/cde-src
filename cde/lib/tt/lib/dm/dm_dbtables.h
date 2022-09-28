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
 * This file contains the declaration of the class _Tt_dbtables which implements
 * an array of pointers to a fixed number of predefined database tables.
 *
 */

#ifndef  _TT_DM_DBTABLES_H
#define  _TT_DM_DBTABLES_H

#include <dm/dm_enums.h>
#include <dm/dm_table_desc.h>
#include <util/tt_object.h>

class _Tt_dbtables : public _Tt_object {    /* array of database tables */
      public:
	_Tt_dbtables();
	virtual ~_Tt_dbtables();
	_Tt_table_desc_ptr	lookup(_Tt_dbtable_id table_id) const;
	void        		insert(_Tt_dbtable_id table_id,
				       _Tt_table_desc_ptr table_desc);
      private:
        _Tt_table_desc_ptr	*table;
};

declare_ptr_to(_Tt_dbtables)

#endif /* _TT_DM_DBTABLES_H */
