/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * dm_buffer.h
 *
 *
 * Full declaration of _Tt_buffer for the few routines that need it.
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 */
#if !defined(_DM_BUFFER_H)
#define _DM_BUFFER_H
#include "util/tt_object.h"
#include "util/tt_list.h"
class _Tt_buffer : public _Tt_object {
      public:
	_Tt_buffer();
	virtual ~_Tt_buffer();
	friend	class _Tt_db_manager;
      private:
	char	_buffer[ISMAXRECLEN];
	int	availp;
};

#endif /* _DM_BUFFER_H */
