/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * @(#)tttkutils.h	1.3 93/09/07
 *
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */
#ifndef ttdtutils_h
#define ttdtutils_h

extern char		_TttkKeys[];

#define			_TttkCBKey		((int)&_TttkKeys[0])
#define			_TttkClientCBKey	((int)&_TttkKeys[1])
#define			_TttkClientDataKey	((int)&_TttkKeys[2])
#define			_TttkDepositPatKey	((int)&_TttkKeys[3])
#define			_TttkJoinInfoKey	((int)&_TttkKeys[4])
#define			_TttkContractKey	((int)&_TttkKeys[5])
#define			_TttkSubContractKey	((int)&_TttkKeys[6])
const int		_TttkNumKeys		= 7;

void			_ttDtPrint(
				const char     *whence,
				const char     *msg
			);
void			_ttDtPrintStatus(
				const char     *whence,
				const char     *expr,
				Tt_status	err
			);
void			_ttDtPrintInt(
				const char     *whence,
				const char     *msg,
				int		n
			);
void			_ttDtPError(
				const char     *whence,
				const char     *msg
			);
const char	       *_ttDtOp(
				Tttk_op		op
			);
Tt_callback_action	_ttDtCallbackAction(
				Tt_message msg
			);

#endif
