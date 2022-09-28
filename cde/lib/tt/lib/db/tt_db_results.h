/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * tt_db_results.h - Defines the result values that may be returned by
 *                   a TT DB server operation.
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 */

#ifndef _TT_DB_RESULTS_H
#define _TT_DB_RESULTS_H

enum _Tt_db_results {
  TT_DB_OK				= 0,
  TT_DB_WRN_FORWARD_POINTER		= 1,
  TT_DB_WRN_SAME_OBJECT_ID		= 2,
  TT_DB_ERR_DB_CONNECTION_FAILED	= 3,
  TT_DB_ERR_DB_OPEN_FAILED		= 4,
  TT_DB_ERR_DB_LOCKED			= 5,
  TT_DB_ERR_RPC_CONNECTION_FAILED	= 6,
  TT_DB_ERR_RPC_FAILED			= 7,
  TT_DB_ERR_CORRUPT_DB			= 8,
  TT_DB_ERR_DISK_FULL			= 9,
  TT_DB_ERR_ILLEGAL_FILE		= 10,
  TT_DB_ERR_ILLEGAL_OBJECT		= 11,
  TT_DB_ERR_ILLEGAL_PROPERTY		= 12,
  TT_DB_ERR_ILLEGAL_MESSAGE		= 13,
  TT_DB_ERR_SAME_FILE			= 14,
  TT_DB_ERR_SAME_OBJECT			= 15,
  TT_DB_ERR_FILE_EXISTS			= 16,
  TT_DB_ERR_OBJECT_EXISTS		= 17,
  TT_DB_ERR_NO_SUCH_FILE		= 18,
  TT_DB_ERR_NO_SUCH_OBJECT		= 19,
  TT_DB_ERR_NO_SUCH_PROPERTY		= 20,
  TT_DB_ERR_ACCESS_DENIED		= 21,
  TT_DB_ERR_NO_ACCESS_INFO		= 22,
  TT_DB_ERR_NO_OTYPE			= 23,
  TT_DB_ERR_OTYPE_ALREADY_SET		= 24,
  TT_DB_ERR_UPDATE_CONFLICT		= 25,
  TT_DB_ERR_PROPS_CACHE_ERROR		= 26,

  // new for filename mapping.  We get this
  // if there is a mild version
  // mismatch, e.g. a libtt talking to
  // a dbserver which doesn't support filename
  // mapping, and one of the new
  // API calls is used.  This is relatively
  // benign, just let the user know that
  // his call is unimplemented in the system
  // he/she tried to contact, so the user can
  // differentiate this from the case where the
  // RPC call failed due to the server not running, etc.
  TT_DB_ERR_RPC_UNIMP			= 27
};

#endif // _TT_DB_RESULTS_H
