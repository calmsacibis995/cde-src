/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_magic.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * This file contains the declaration for class _Tt_magic which implements the
 * magic string in every NetISAM database.  This magic string contains the
 * database version number and other things.
 *
 */

#ifndef  _TT_DM_MAGIC_H
#define  _TT_DM_MAGIC_H
#include <util/tt_object.h>

class _Tt_magic : public _Tt_object {
      public:
	_Tt_magic();
	~_Tt_magic();
	int		versionnumber();
	void		setverno(int ver_no);
	char		*magicstring();
      private:
	char		*magic_string;
};

#endif  /* _TT_DM_MAGIC_H */
