//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 * Tool Talk Database Manager (DM) - dm_transaction.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * This file contains implementation of _Tt_trans_list class.
 *
 */

#include <dm/dm.h>
#include <dm/dm_transaction.h>
#include <util/tt_string.h>
#include <memory.h>


_Tt_trans_list::
~_Tt_trans_list()
{
	if (DM_DEBUG(5)) {
		printf("~_Tt_trans_list: deleting transaction records list . . .\n");
		print();
	}
	_Tt_trans_record *tr = _head;
	_Tt_trans_record *d;
	while (tr) {
		d = tr;
		tr = tr->next;
		free(d->rec.rec_val);
		free((MALLOCTYPE *)d);
	}
}

void _Tt_trans_list::
add(_Tt_record_ptr rec)
{
	if (DM_DEBUG(5)) {
		printf("_Tt_trans_list::add - rec ");
		rec->print();
	}
	_Tt_trans_record *trec =
	  (_Tt_trans_record *)malloc(sizeof(_Tt_trans_record));
	trec->newp = rec->newrecp();
	trec->recnum = rec->record_num();
	trec->rec.rec_len = rec->rec_len;
	trec->rec.rec_val = (char *)malloc(rec->rec_len);
	memcpy(trec->rec.rec_val, rec->bufadr(), rec->rec_len);
	trec->next = _head;
	_head = trec;
}

void _Tt_trans_list::
print(FILE *fs) const
{
	_Tt_trans_record *trec = _head;
	while (trec) {
		fprintf(fs, "_Tt_trans_list::print - trec - newp = %d, recnum = %d, rec_len = %d\n", trec->newp, trec->recnum, trec->rec.rec_len);
		trec = trec->next;
	}
}

implement_ptr_to(_Tt_trans_list)

