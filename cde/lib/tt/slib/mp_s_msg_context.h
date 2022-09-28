/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* -*-C++-*-
 *
 * @(#)mp_s_msg_context.h	1.3 29 Jul 1993
 *
 * Tool Talk Message Passer (MP) - mp_s_msg_context.h
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 *
 * _Tt_s_msg_context knows the client side of the context RPC interface.
 */

#ifndef _MP_S_MSG_CONTEXT_H
#define _MP_S_MSG_CONTEXT_H

#include <mp/mp_msg_context.h>
#include <mp/mp_session.h>
#include <mp/mp_procid_utils.h>

class _Tt_s_msg_context : public _Tt_msg_context {
      public:
	_Tt_s_msg_context();
	virtual ~_Tt_s_msg_context();

	Tt_status		s_join(
					_Tt_procid_ptr &procID
				) const;
	Tt_status		s_quit(
					_Tt_procid_ptr &procID
				) const;
};

#endif				/* _MP_S_MSG_CONTEXT_H */
