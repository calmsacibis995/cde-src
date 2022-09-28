/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * This file contains the declaration of class _Tt_db_manager which implements
 * the database manager (DM).
 *
 */

#ifndef  _TT_DM_H
#define  _TT_DM_H

#include "tt_options.h"
#include <dm/dm_enums.h>
#include <dm/dm_db.h>
#include <dm/dm_dbtables.h>
#include <dm/dm_dbdescs_table.h>
#include <dm/dm_dbrefs_table.h>
#include <dm/dm_exports.h>
#include <dm/dm_dbref.h>
#include <dm/dm_key.h>
#include <dm/dm_key_desc.h>
#include <dm/dm_mfs.h>
#include <dm/dm_redirect.h>
#include <dm/dm_rpc_mount.h>
#include <util/tt_object.h>
#include <netdb.h>
#include <sys/param.h>
#include "tt_options.h"
#if defined(OPT_BUG_SUNOS_4) && defined(__GNUG__)
#	include <stdlib.h>
#endif
#include <rpc/rpc.h>

#if !defined(_TT_NODEBUG)
#define DM_DEBUG(n) (_tt_dm->debug_flag>=(n))
#else
#define DM_DEBUG(n) 0
#endif


#define _TT_MFS_BUCKETS  19

class _Tt_record;
class _Tt_db_manager;
class _Tt_buffer;
declare_list_of(_Tt_buffer)

class _Tt_server : public _Tt_object {
      public:
	_Tt_server();
	_Tt_server(char *h, CLIENT *s);
	virtual ~_Tt_server();
	_Tt_string	&host() { return _host; }
	CLIENT		*server() { return _server; }
      private:
	_Tt_string	_host;
	CLIENT		*_server;
};

declare_list_of(_Tt_server)
declare_table_of(_Tt_server)


/*
 *  The database manager class
 */

class _Tt_db_manager : public _Tt_object {
      public:
	_Tt_db_manager();
	virtual ~_Tt_db_manager();
	_Tt_dm_status	init();
	_Tt_dm_status	docpath_to_dbref(_Tt_string path,
					 _Tt_dbref_ptr &dbref);
	_Tt_string	resolve_path(_Tt_string path);
	_Tt_string	resolve_abspath(_Tt_string abspath);
	void		set_auth(int flavor) { auth_flavor = flavor; }
	int		get_auth() { return auth_flavor; }
	uid_t		uid();
	int		num_groups();
	gid_t	       *gids();
	_Tt_dm_status	error_code() { return errcode; }
	_Tt_dm_status	error_string();
	int		debug_flag;
	friend	class _Tt_db;
	friend	class _Tt_db_desc;
	friend  class _Tt_dbref;
	friend  class _Tt_dbrefs_table;
	friend  class _Tt_key;
	friend  class _Tt_link_ends_record;
	friend  class _Tt_oid;
	friend	class _Tt_record;
	friend	class _Tt_table_desc;
      private:
	_Tt_string	default_relpath(_Tt_string abspath);
	char		*get_buffer();
	void		free_buffer(const char * b);
	_Tt_dm_status	create_dbdesc(_Tt_dbref_ptr dbref,
				      _Tt_dbtable_id id,
				      _Tt_db_desc_ptr &dbdesc);
	_Tt_dm_status	nfs_path_to_dbref(_Tt_string path, _Tt_dbref_ptr &dbref);
	void		set_real_path(_Tt_string path);
	void		lookup_redirection();
	MNTENT		*lookup_mfs(int dirp, const char *dir, int *longest_index = 0);
	MNTENT		*cache_lookup_mfs(int dirp, const char *dir,
					  int *longest_index) const;
	_Tt_dm_status	update_mfs_info();
	int		hash_mfs(const char *s) const;
	int		hash_mfs(const char *s, int s_len) const;
	void		insert_mfs(const MNTENT *e);
	void		delete_mfs(const MNTENT *e);
	char		*lookup_export(const char *host, const char *path);
	void		delete_dbdesc(_Tt_db_desc *dbdesc);
	int		num_isfds() { return dbdescs->num_isfds(); }
	void		inc_num_isfds() { dbdescs->inc_num_isfds(); }
	void		dec_num_isfds() { dbdescs->dec_num_isfds(); }
	long		host_id() { return _hostid; }
	_Tt_dbref_ptr	create_dbref(_Tt_string &ns, _Tt_string &env,
				     _Tt_string &path);
	_Tt_dm_status	create_dbref(_Tt_string &dbrefstr,
				     _Tt_dbref_ptr &dbref);
	_Tt_dm_status		errcode;
	_Tt_name_space		namespace;
	_Tt_dbref_ptr		_dbref;   /* for NSE, USER name spaces */
	char			rhost[MAXHOSTNAMELEN];	/* for mapping */
	char			host_name[MAXHOSTNAMELEN];
	long			_hostid;
        _Tt_dbtables_ptr	dbtables;
	_Tt_dbdescs_table_ptr	dbdescs;
	_Tt_dbrefs_table_ptr	dbrefs;
	_Tt_key_desc_ptr	no_key_desc;
	int			auth_flavor;
	uid_t			user_id;
	_Tt_buffer_list_ptr	buffers;
	/* mounted file systems cache */
	_Tt_mfs_list_ptr	mfs_table;
	char			servernetname[MAXNETNAMELEN];
	struct	sockaddr_in	server_addr;
	_Tt_server_table_ptr	server_table;
	char			real_path[MAXPATHLEN];
	char			resolved_path[MAXHOSTNAMELEN+1+MAXPATHLEN];
	time_t			mtab_last_mtime;
	_Tt_redirect_table_ptr	redirect_table;
};

/*
 *  The global variable pointing to the Database Manager object.
 */

extern _Tt_db_manager *_tt_dm;
extern _Tt_dm_status _tt_dm_init();
extern void _tt_dm_close();

#endif /* _TT_DM_H */
