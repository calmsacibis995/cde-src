/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* -*-C++-*-
 * Tool Talk Database Manager (DM)
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * This file contains the class declaration for a generic database record which
 * all specific database record types inherit from.
 *
 */

#ifndef  _TT_DM_RECORD_H
#define  _TT_DM_RECORD_H

#include <dm/dm_enums.h>
#include <dm/dm_table_desc.h>     
#include <util/tt_object.h>
#include <util/tt_string.h>     

// The high-order bit in fields_set is used to keep track of
// _recnum being set, which indicates retrieval by record
// number instead of key.
const int FLD_REC_NUM = 0x8000000;

class _Tt_db_desc;
class _Tt_node;
class _Tt_table_desc;
class _Tt_trans_list;

class _Tt_record : public _Tt_object {
      public:
	_Tt_record();
	virtual ~_Tt_record();
	void		reset();
	void            setold();
	void		setnew();
	int             record_num() const;
	void            set_record_num(long int rec_num);
	void		print(FILE *fs = stdout) const;
	// following two ought to be pure virtual, but tt_ptr package cannot
        // handle abstract classes.
	virtual char *	bufadr();
	virtual const char *bufadr() const;
	virtual void	determine_key();
	_Tt_string	extract_key(_Tt_table_desc_ptr table);
	int             reclength() const;
	void            set_reclength(int len);
	// Why do all these friends exist? 
	friend	class _Tt_db_desc;
	friend	class _Tt_node;
	friend	class _Tt_table_desc;
	friend  class _Tt_trans_list;
      protected:
	// fields_set is a bit map that keeps track of which record
	// fields have been set.  This restricts us to only 31 fields
	// per record (since _recnum counts as a field),
	// but the most we have currently is 4.
	int		fields_set;
	int        	newrecp() const;
	void		set_version_num(int ver_no);
	int		version_num() const;
	void		set_recnum(long recnum);

	_Tt_key_id	key_id;
	short		key_len;
        long int        _recnum;
        short           rec_len;
	int         	newp;
};

declare_ptr_to(_Tt_record)

#endif  /* _TT_DM_RECORD_H */
