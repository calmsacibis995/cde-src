/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * prop.h
 *
 * Part of the inspect&repair tools.
 *
 * accumulates, prints, rewrites(?) data for a spec or link.
 * This is for the sole use of dbck, so much less information
 * hiding is done than the usual TT class definition style:
 * in particular, data members are simply made public,
 * instead of defining access functions.
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#ifndef _PROP_H
#define _PROP_H

#include "util/tt_object.h"
#include "util/tt_list.h"
#include "util/tt_table.h"
#include "util/tt_string.h"

declare_list_of(Prop)
declare_table_of(Prop)

class Prop : public _Tt_object {
      public:
        Prop(){};
	Prop(_Tt_string propname);
	~Prop() {};
	virtual void print(FILE * f) const;
	_Tt_string	&name() { /* key access for table package */
		return _name;
	}
	_Tt_string		_name;
	_Tt_string_list_ptr	_values;
};
	
_Tt_string _tt_prop_name(_Tt_object_ptr &o);

#endif /* _SPEC_H */
