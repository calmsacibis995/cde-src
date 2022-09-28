/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* -*-C++-*-
 *
 * @(#)mp_c_msg_context.h	1.4 30 Jul 1993
 *
 * Tool Talk Message Passer (MP) - mp_c_msg_context.h
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 *
 * _Tt_c_msg_context knows the client side of the context RPC interface.
 */

#ifndef _MP_C_MSG_CONTEXT_H
#define _MP_C_MSG_CONTEXT_H

#include <mp/mp_msg_context.h>
#include <mp/mp_c_msg_context_utils.h>
#include <mp/mp_session.h>
#include <mp/mp_procid_utils.h>

class _Tt_c_msg_context : public _Tt_msg_context {
      public:
	_Tt_c_msg_context();
	virtual ~_Tt_c_msg_context();

	Tt_status		c_join(
					_Tt_session &session,
					_Tt_procid_ptr &procID
				);
	Tt_status		c_quit(
					_Tt_session &session,
					_Tt_procid_ptr &procID
				);
};

#endif				/* _MP_C_MSG_CONTEXT_H */
