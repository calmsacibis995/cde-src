/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* @(#)dm_exports.h	1.10 93/09/07
 * Tool Talk Database Manager (DM) - dm_exports.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Declaration of class _Tt_exports which implements a linked element
 * containing the export list (/etc/xtab) of a remote host.
 *
 */

#ifndef  _TT_DM_EXPORTS_H
#define  _TT_DM_EXPORTS_H

#include <dm/dm_rpc_mount.h>
#include <util/tt_string.h>
#include <util/tt_object.h>
#include <util/tt_list.h>

class _Tt_exports : public _Tt_object {
      public:
	_Tt_exports() {}
	_Tt_exports(const char *host, exports *exl);
	virtual ~_Tt_exports();
	_Tt_string	&host();
	void		print(FILE *fs = stdout) const;
      private:
	_Tt_string	_host;
	struct exports	*_exps;
};

declare_list_of(_Tt_exports)

#endif /* _TT_DM_EXPORTS_H */
