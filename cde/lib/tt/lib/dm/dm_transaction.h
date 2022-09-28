/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */

/*
 * Tool Talk Database Manager (DM) - dm_transaction.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * This file contains the declaration of _Tt_trans_list implementing a linked
 * list of transaction records.
 *
 */

#ifndef  _TT_DM_TRANSACTION_H
#define  _TT_DM_TRANSACTION_H

#include "db/db_server.h"
#include "dm/dm_record.h"

class _Tt_trans_list : public _Tt_object {
      public:
	_Tt_trans_list() { _head = 0; }
	virtual ~_Tt_trans_list();
	void			add(_Tt_record_ptr rec);
	_Tt_trans_record_list	trans_records() { return _head; }
	void			print(FILE* fs = stdout) const;
      private:
	_Tt_trans_record_list	_head;
};

declare_ptr_to(_Tt_trans_list)

#endif /* _TT_DM_TRANSACTION_H */
