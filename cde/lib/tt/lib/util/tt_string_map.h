/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* @(#)tt_string_map.h	1.4 @(#)
 * Tool Talk Utility - tt_string_map.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Declares a class for holding string maps.  This
 * class is used for 2 different things at this time:
 *
 *	_Tt_db_hostname_redirection_map
 *	_Tt_db_partition_redirection_map
 */

#ifndef _TT_STRING_MAP_H
#define _TT_STRING_MAP_H

#include "util/tt_map_entry_utils.h"
#include "util/tt_object.h"
#include "util/tt_string.h"
#include "util/tt_table.h"

class _Tt_string_map : public _Tt_object {
public:
  _Tt_string_map (_Tt_object_table_keyfn key_function);
  _Tt_string_map ();

  ~_Tt_string_map ();

  void loadFile (const _Tt_string &file);
  
  _Tt_string findEntry(const _Tt_string &address);

private:
  _Tt_map_entry_table_ptr mapEntries;
};

#endif /* _TT_STRING_MAP_H */
