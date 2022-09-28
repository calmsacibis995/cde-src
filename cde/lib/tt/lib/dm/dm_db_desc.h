/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_db.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 *  The class _Tt_db implements a descriptor that contains information
 *  about a database's location and its table layout.
 */

#ifndef  _TT_DM_DB_DESC_H
#define  _TT_DM_DB_DESC_H

#include "dm/dm_dbref.h"
#include "dm/dm_key_desc.h"
#include "dm/dm_magic.h"
#include "dm/dm_oid.h"
#include "dm/dm_record.h"
#include "dm/dm_table_desc.h"
#include "util/tt_object.h"
#include "util/tt_list.h"
#include "util/tt_string.h"

#include <netinet/in.h>

#define _TT_DBCLIENT_SIDE
#include "db/db_server.h"
#include "dm/dm_transaction.h"
extern int iserrno;

class _Tt_db_manager;
class _Tt_dbdescs_table;

class _Tt_db_desc : public _Tt_object {
      public:
	_Tt_db_desc();
	_Tt_db_desc(_Tt_dbref_ptr dbref, _Tt_string dbname);
	virtual ~_Tt_db_desc();
	_Tt_dm_status		open();
	void			reset_read() { read_key_id = DM_KEY_UNDEFINED; }
	_Tt_dm_status		read(_Tt_record_ptr rec);
	_Tt_dm_status		readfirst(_Tt_record_ptr rec);
	_Tt_dm_status		readnext(_Tt_record_ptr rec);
	_Tt_dm_status		test_and_set(_Tt_record_ptr rec);
	_Tt_dm_status		write(_Tt_record_ptr rec);
	_Tt_dm_status		transaction_write(_Tt_record_ptr rec);
	_Tt_dm_status		transaction_abort();
	_Tt_dm_status		transaction_commit();
	_Tt_dm_status		remove(_Tt_record_ptr rec);
	_Tt_dm_status		close();
	_Tt_dm_status		setuid(_Tt_oid_ptr oid, uid_t uid);
	_Tt_dm_status		setgroup(_Tt_oid_ptr oid, gid_t group);
	_Tt_dm_status		setmode(_Tt_oid_ptr oid, mode_t mode);
	_Tt_dm_status		getaccess(_Tt_oid_ptr oid, uid_t &uid,
					  gid_t &group, mode_t &mode);
#ifdef UNUSED
	_Tt_dm_status		readspec(_Tt_oid_ptr oid,
					 _Tt_prop_array &props);
	_Tt_dm_status		writespec(_Tt_oid_ptr oid,
					  _Tt_prop_array props);
	_Tt_dm_status		addsession(_Tt_oid_ptr oid,
					   _Tt_string session);
	_Tt_dm_status		delsession(_Tt_oid_ptr oid,
					   _Tt_string session);
#endif /* UNUSED */	
	_Tt_dm_status		gettype(_Tt_oid_ptr oid, _Tt_string typeprop,
					_Tt_string &typename);
	_Tt_dm_status		error_code() { return errcode; }
	_Tt_string		error_string();
	friend	class _Tt_db_manager;
	friend  class _Tt_dbdescs_table;
	friend	class _Tt_db;
      private:
	_Tt_dbtable_id		tableid() const;
	_Tt_dm_status		create();
	void            	settable(_Tt_table_desc_ptr table);
	_Tt_dbref_ptr		dbref();
	_Tt_name_space		namespace() const;
	_Tt_string		envname();
	_Tt_string		host();
	_Tt_string		path();
	_Tt_string		dbname();
	_Tt_key_desc_ptr	findkeydesc(_Tt_key_id key_id) const;
	int	         	variable_recp() const;

	CLIENT			*server;
	int			auth_level;
	char			servernetname[MAXNETNAMELEN];
	struct	sockaddr_in	server_addr;
	_Tt_trans_list_ptr	_trecs;	/* transaction records buffer */
	void			create_auth();
	static	long		isrecnum;    /* NetISAM wrappers for db server */
	static	int		isreclen;
	int			isaddindex(int, struct keydesc*);
	int			isbuild(char*, int, struct keydesc*, int);
	int			isclose(int);
	int			iscntl(int, int, char*);
	int			isdelrec(int, long, char*);
	int 			iserase(char*);
	int 			isopen(char*, int);
	int 			isread(int, char*, int);
	int 			isrewrec(int, long, char*);
	int 			isstart(int, struct keydesc*, int, char*, int);
	int 			iswrite(int, char*);

	static	_Tt_dm_status	errcode;
	_Tt_dbref_ptr		_dbref;
	_Tt_string		_dbname;
	_Tt_magic		magic;
	_Tt_table_desc_ptr	table;
	int             	isfd;		/* NetISAM file descriptor */
	_Tt_key_id		read_key_id;	/* key id for read iterator */
	int			num_opens;
	_Tt_string		saved_key;	/* Key used on last read */
};

declare_list_of(_Tt_db_desc)

#endif /* _TT_DM_DB_DESC_H */
