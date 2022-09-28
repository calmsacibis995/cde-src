/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * dm_path.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * The routines which map relative paths to absolute paths and back
 */

#ifndef _DM_PATH_H
#define _DM_PATH_H

#include <util/tt_string.h>

_Tt_dm_status _tt_abs_path_of(_Tt_string rel_path, _Tt_string &abs_path);
_Tt_dm_status _tt_rel_path_of(_Tt_string abs_path, _Tt_string &rel_path);

#endif /* _DM_PATH_H */
