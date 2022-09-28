/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* @(#)tt_file_system_entry.h	1.9 93/09/07
 * Tool Talk Utility - tt_file_system_entry.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Declares a class representing an entry in the file system mount table.
 *
 */

#ifndef  _TT_FILE_SYSTEM_ENTRY_H
#define  _TT_FILE_SYSTEM_ENTRY_H

#include "util/tt_object.h"
#include "util/tt_string.h"

class _Tt_file_system_entry : public _Tt_object {
    public:
				_Tt_file_system_entry(
					const _Tt_string &hostname,
					const _Tt_string &mountpt,
					const _Tt_string &partition,
					int		  islocal,
					int		  isloopback,
					int		  iscachefs
				);
				_Tt_file_system_entry();
				~_Tt_file_system_entry();

	_Tt_string      getHostname()	const { return entryHostname; }
	_Tt_string      getMountPoint()	const { return entryMountPoint; }
	_Tt_string      getPartition()	const { return entryPartition; }
	_Tt_string      getLoopBackMountPoint() const {
				return loopBackMountPoint;
	    		}
	bool_t		isLocal() const { return localFlag; }
	bool_t		isLoopBack() const{ return loopBackFlag; }
	bool_t		isCacheFS() const{ return cachefsFlag; }

    private:
	_Tt_string		entryHostname;
	_Tt_string		entryMountPoint;

	// Now called "special" in Solaris 2.6 doc.
	_Tt_string		entryPartition;

	_Tt_string		loopBackMountPoint;
	bool_t			localFlag;
	bool_t			loopBackFlag;

	// If this is a cachefs entry we have to backmap to
	// find if what's cached here is local, or remote,
	// so file scoped messaging and filename mapping
	// can find the correct dbserver.

	// This is the "entryMountPoint" half of a cachefs entry.
	bool_t			cachefsFlag;	

    friend class _Tt_file_system;
};

#endif /* _TT_FILE_SYSTEM_ENTRY_H */
