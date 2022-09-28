/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * archiver.h - Interface to archiver, an LS/TT-aware tar(1).
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 */

#ifndef _ARCHIVER_H
#define _ARCHIVER_H

#include <api/c/tt_c.h>
#include <util/tt_string.h>
#include "tttar_string_map.h"

/*
 * Type definitions
 */

typedef enum archive_mode {
	NO_MODE,
	CREATE,
	LIST,
	EXTRACT} Archive_mode;

class archiver : public _Tt_object {
    public:
	archiver( char *arg0 );
	virtual ~archiver();

	int			do_tar( _Tt_string tttardir,
				        bool_t only_tttarfile );
	bool_t			do_tttar( char *tttarfile_name,
					  bool_t silent );
	void			parse_args( int argc, char **argv );
	void			usage( FILE *fs = stderr ) const;
	_Tt_string		process_name() { return _process_name; }
	Archive_mode		mode() { return _mode; }
	void			mode_set( Archive_mode m ) { _mode = m; }
	bool_t			verbosity() { return _verbosity; }
	void			verbosity_set( int v ) { _verbosity = v; }
	bool_t			should_tar() { return _should_tar; }
	bool_t			only_1_look_at_tarfile()
					{ return _only_1_look_at_tarfile; }
	_Tt_string		tarfile_arg() { return _tarfile_arg; }

    private:
	FILE		       *_open_io_stream( char *filename,
					        bool_t complain );

	_Tt_string		_process_name;
	_Tt_string		_prog_name;
	Archive_mode		_mode;
	int			_verbosity;
	bool_t			_follow_symlinks;
	bool_t			_recurse;
	bool_t			_archive_links;
	bool_t			_archive_external_links;
	bool_t			_preserve_modes;
	bool_t			_preserve__props;
	bool_t			_should_tar;
	bool_t			_only_1_look_at_tarfile;
	FILE		       *_io_stream;
	_Tt_string		_tarfile_arg;
	_Tt_string_list_ptr	_paths2tar;
	Lstar_string_map_list_ptr _renamings;
};

#endif /* _ARCHIVER_H */
