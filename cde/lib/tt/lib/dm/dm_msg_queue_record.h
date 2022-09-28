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
 * The class _Tt_msg_queue_record extends the _Tt_record type to provide the
 * behavior of an MSG_QUEUE record.
 */

#ifndef  _TT_DM_MSG_QUEUE_RECORD_H
#define  _TT_DM_MSG_QUEUE_RECORD_H

#include <dm/dm_record.h>
#include <dm/dm_key.h>
#include <util/tt_string.h>

struct Table_msg_queue;

class _Tt_msg_queue_record : public _Tt_record {
      public:
	_Tt_msg_queue_record();
	virtual ~_Tt_msg_queue_record();
	_Tt_key_ptr	dockey() const;
	void		set_dockey(_Tt_key_ptr key);
	int		id() const;
	void		set_id(int id);
	int		part() const;
	void		set_part(int part);
	_Tt_string	body() const;
	void		set_body(_Tt_string body);
	int		max_body_length() const;
	void		print(FILE *fs = stdout) const;
	virtual void	determine_key();
	virtual char *	bufadr();
	virtual const char *bufadr() const;
      private:
	Table_msg_queue *field;
};

declare_derived_ptr_to(_Tt_msg_queue_record,_Tt_record)

#endif  /* _TT_DM_MSG_QUEUE_RECORD_H */
