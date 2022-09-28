//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*  @(#)dm_db_desc.C	1.30 93/09/07
 *  Tool Talk Database Manager - dm_db_desc.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  This file contains the implementation of the _Tt_db member functions.
 */

#include "tt_options.h"
#include "dm/dm_db_desc.h"
#include "dm/dm_oid_prop_record.h"
#include "dm/dm_oid_container_record.h"
#include "dm/dm_docoid_path_record.h"
#include "dm/dm_msg_queue_record.h"
#include "dm/dm.h"
#include "util/tt_assert.h"
#include "util/tt_port.h"
#include "util/tt_gettext.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <isam.h>     
#include <netdb.h>
#include <sys/socket.h>
#include <memory.h>

#if defined(OPT_BUG_SUNOS_4) && defined(__GNUG__)
	extern "C" { int endhostent(); }
#endif
// Some old versions of RPC headers don\'t define AUTH_NONE but
// do define the older AUTH_NULL flavor.

#if !defined(AUTH_NONE)
#define AUTH_NONE AUTH_NULL
#endif

const char *RPC_PROTO = "tcp";
const int CRED_EXPIRE = 10;
static struct timeval NORMAL_TIMEOUT = { 1000000, 0 };
static struct timeval QUICK_TIMEOUT = {4,0};

#if defined(ultrix)
extern "C" clnt_spcreateerror(char *);
#endif

int iserrno = 0;
long _Tt_db_desc::isrecnum = 0;
int _Tt_db_desc::isreclen = 0;

void _Tt_db_desc::
create_auth()
{
#if defined(OPT_SECURE_RPC)
	/* generate DES auth credential */
	if (auth_level == AUTH_DES) {
		if (server->cl_auth) {
			auth_destroy(server->cl_auth);
		}
#if defined(OPT_TLI)
		server->cl_auth = authdes_seccreate(servernetname,
						    CRED_EXPIRE,
						    servernetname, 0);
#else
		server->cl_auth = authdes_create(servernetname,
						 CRED_EXPIRE,
						 &server_addr, 0);
#endif /* OPT_TLI */
	}
#endif /* OPT_SECURE_RPC */
}

int _Tt_db_desc::
isaddindex(int isfd, struct keydesc *key)
{
	create_auth();
	_Tt_isaddindex_args args;
	args.isfd = isfd;
	args.key = key;
	_Tt_isam_results *isresult = _tt_isaddindex_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	return isresult->result;
}

int _Tt_db_desc::
isbuild(char *path, int reclen, struct keydesc *key, int mode)
{
	create_auth();
	_Tt_isbuild_args args;
	args.path = path;
	args.reclen = reclen;
	args.key = key;
	args.mode = mode;
	args.isreclen = isreclen;
	_Tt_isam_results *isresult = _tt_isbuild_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	return isresult->result;
}

int _Tt_db_desc::
isclose(int isfd)
{
	create_auth();
	_Tt_isam_results *isresult = _tt_isclose_1(&isfd, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	return isresult->result;
}

int _Tt_db_desc::
iscntl(int isfd, int func, char *arg)
{
	create_auth();
	_Tt_iscntl_args args;
	args.isfd = isfd;
	args.func = func;
	args.arg.arg_len = ISAPPLMAGICLEN;
	args.arg.arg_val = arg;
	_Tt_iscntl_results *isresult = _tt_iscntl_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	memcpy(arg, isresult->arg.arg_val, isresult->arg.arg_len);
	if (isresult->arg.arg_val != 0) {
		xdr_free((xdrproc_t)xdr__Tt_iscntl_results, (char *)isresult);
	}
	return isresult->result;
}

int _Tt_db_desc::
isdelrec(int isfd, long recnum, char *rec)
{
	create_auth();
	_Tt_isdelrec_args args;
	args.isfd = isfd;
	args.recnum = recnum;
	args.rec.rec_len = isreclen;
	args.rec.rec_val = rec;
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::isdelrec - isfd = %d, rec_len = %d\n",
		        args.isfd, args.rec.rec_len);
	}
	_Tt_isam_results *isresult = _tt_isdelrec_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::isdelrec - result = %d, iserrno = %d\n",
		       isresult->result, iserrno);
	}
	return isresult->result;
}

int _Tt_db_desc::
iserase(char *path)
{
	create_auth();
	_Tt_isam_results *isresult = _tt_iserase_1(&path, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	return isresult->result;
}

int _Tt_db_desc::
isopen(char *path, int mode)
{
	create_auth();
	_Tt_isopen_args args;
	args.path = path;
	args.mode = mode;
	_Tt_isam_results *isresult = _tt_isopen_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	return isresult->result;
}

int _Tt_db_desc::
isread(int isfd, char *rec, int mode)
{
	create_auth();
	_Tt_isread_args args;
	args.isfd = isfd;
	args.rec.rec_len = isreclen;
	args.rec.rec_val = rec;
	args.mode = mode;
	args.isrecnum = isrecnum;
	_Tt_isread_results *isresult = _tt_isread_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	isreclen = isresult->isreclen;
	isrecnum = isresult->isrecnum;
	iserrno = isresult->isresult.iserrno;
	if (isresult->isresult.result != -1) {
		memcpy(rec, isresult->rec.rec_val, isresult->isreclen);
	}
	if (isresult->rec.rec_val != 0) {
		xdr_free((xdrproc_t)xdr__Tt_isread_results, (char *)isresult);
	}
	return isresult->isresult.result;
}

int _Tt_db_desc::
isrewrec(int isfd, long recnum, char *rec)
{
	create_auth();
	_Tt_isrewrec_args args;
	args.isfd = isfd;
	args.recnum = recnum;
	args.rec.rec_len = isreclen;
	args.rec.rec_val = rec;
	if (DM_DEBUG(5)) {
		fprintf(stderr,
			"_Tt_db_desc::isrewrec - isfd = %d, rec_len = %d\n",
		       args.isfd, args.rec.rec_len);
	}
	_Tt_isam_results *isresult = _tt_isrewrec_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	return isresult->result;
}

int _Tt_db_desc::
isstart(int isfd, keydesc *key, int key_len, char *rec, int mode)
{
	create_auth();
	_Tt_isstart_args args;
	args.isfd = isfd;
	args.key = key;
	args.key_len = key_len;
	args.rec.rec_len = rec ? isreclen : 0;
	args.rec.rec_val = rec ? rec : 0;
	args.mode = mode;
	if (DM_DEBUG(5)) {
		fprintf(stderr, "_Tt_db_desc::isstart: isfd = %d,"
			" key_len = %d, rec.rec_len = %d, mode = 0x%X\n",
			args.isfd, args.key_len, args.rec.rec_len, args.mode);
		fprintf(stderr,"-*-key desc <flags = %d, num parts = %d>-*-\n",
			args.key->k_flags, args.key->k_nparts);
		_Tt_string s((unsigned char*)rec, key_len);
		fprintf(stderr,"\nkey = ");
		s->print(stderr);
		fprintf(stderr,"\n");
	}

	_Tt_isam_results *isresult = _tt_isstart_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	return isresult->result;
}

int _Tt_db_desc::
iswrite(int isfd, char *rec)
{
	create_auth();
	_Tt_iswrite_args args;
	args.isfd = isfd;
	args.rec.rec_len = isreclen;
	args.rec.rec_val = rec;
	_Tt_isam_results *isresult = _tt_iswrite_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		return -1;
	}
	iserrno = isresult->iserrno;
	return isresult->result;
}

/*
 *  _Tt_db_desc::transaction_write - \'write\' a record into the transaction
 *  records buffer.  If succeeds returns DM_OK, otherwise returns DM_ERROR.
 */

_Tt_dm_status _Tt_db_desc::
transaction_write(_Tt_record_ptr rec)
{
	if (isfd == -1) {
		return (errcode = DM_ERROR);
	}
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::transaction_write - transaction write record\n");
		rec->print(stderr);
	}
	_trecs->add(rec);
	return (errcode = DM_OK);
}

/*
 *  _Tt_db_desc::transaction_abort - Abort the transaction by releasing all
 *  records kept in the transaction buffer.  If succeeds returns DM_OK, otherwise
 *  returns DM_ERROR.
 */

_Tt_dm_status _Tt_db_desc::
transaction_abort()
{
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::transaction_abort - aborting . . .\n");
		_trecs->print(stderr);
	}
	_Tt_trans_list *l = _trecs.c_pointer();
	if (l) {
		delete l;
	}
	_trecs = new _Tt_trans_list();
	return (errcode = DM_OK);
}


/*
 *  _Tt_db_desc::transaction_commit - sends all the transaction records in
 *  the transaction records buffer to the db server.  If the transaction
 *  succeeds returns DM_OK and clears the transaction buffer, otherwise returns
 *  DM_ERROR.
 */

_Tt_dm_status _Tt_db_desc::
transaction_commit()
{
	create_auth();
	if (isfd == -1) {
		return (errcode = DM_ERROR);
	}
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::transaction_commit - committing transaction . . .\n");
		_trecs->print();
	}
	_Tt_trans_record_list recs = _trecs->trans_records();
	if (!recs) {
		return (errcode = DM_ERROR);
	}
	_Tt_transaction_args args;
	args.isfd = isfd;
	args.recs = recs;
	_Tt_isam_results *isresult = _tt_transaction_1(&args, server);
	if (!isresult) {
		iserrno = ERPC;
		transaction_abort();	/* clears buffer */
		return (errcode = DM_ERROR);
	}
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::transaction_commit - transaction result = %d, iserrno = %d\n", isresult->result, isresult->iserrno);
	}
	iserrno = isresult->iserrno;
	transaction_abort();	/* clears buffer */
	if (isresult->result == -1) {
		return (errcode = DM_ERROR);
	} else {
		return (errcode = DM_OK);
	}
}

_Tt_dm_status _Tt_db_desc::errcode = DM_OK;

_Tt_db_desc::
_Tt_db_desc()
{
}

_Tt_db_desc::
_Tt_db_desc(_Tt_dbref_ptr dbref, _Tt_string dbname)
{
	clnt_stat rpc_status;
	_dbref = dbref;
	_dbname = (char *) dbname;	/* copy */
	isfd = -1;
	read_key_id = DM_KEY_UNDEFINED;
	num_opens = 0;
	
	if (DM_DEBUG(5)) {
		fprintf(stderr,
			"_Tt_db_desc::_Tt_db_desc: dbref is\n");
		dbref->print(stderr);
		fprintf(stderr,
			"_Tt_db_desc::_Tt_db_desc: _dbref is\n");
		_dbref->print(stderr);
	}


	/* setup server rpc handle and get minimum authentication level info */

	/* This is always the first RPC call to a given server.  This means  */
	/* that it may cause inetd to start the server.  Because of the way  */
	/* inetd works, the RPC call that starts the server always times     */
	/* out.  So use a special quick time out for the first call.         */


	int non_responding_message_given = 0;
	int *result = 0;
	for (int i=0; i<10; i++) {
                server = clnt_create((char *)_dbref->envname,
                                     TT_DBSERVER_PROG, TT_DBSERVER_VERS,
                                     (char *)RPC_PROTO);
		if (server) {
			char *path = "";
			clnt_control(server, CLSET_TIMEOUT,
				     (char *)&QUICK_TIMEOUT);

			result = _tt_min_auth_level_1(&path,
						      server,
						      &rpc_status);
			if (result || rpc_status!=RPC_TIMEDOUT) {
				break;
			}
			/* 
			 * Since the first call is very likely to time out,
			 * don\'t complain that time.
			 */
			if (i!=0) {
				_tt_syslog(0, LOG_ERR,
					   catgets(_ttcatd, 1, 9,
						   "rpc.ttdbserverd on %s not "
						   "responding, still trying"),
					   (char *)_dbref->envname);
				non_responding_message_given = 1;
			}
			clnt_destroy(server);
			server = 0;
		} else {
			_tt_syslog(0, LOG_ERR,
				   catgets(_ttcatd, 1, 10,
					   "clnt_create for rpc.ttdbserverd on "
					   "host %s failed%s"),
				   (char *)_dbref->envname,
				   clnt_spcreateerror(""));
			return;
		}
	}
	
	if (!result) {
		_tt_syslog(0, LOG_ERR,
			   catgets(_ttcatd, 1, 11,
				   "Error: rpc.ttdbserverd "
				   "on %s is not running"),
			   (char *)_dbref->envname);
		if (server) {
			clnt_destroy(server);
			server = 0;
			return;
		}
	}
	clnt_control(server, CLSET_TIMEOUT, (char *)&NORMAL_TIMEOUT);
	if (non_responding_message_given) {
		_tt_syslog(0, LOG_ERR,
			   catgets(_ttcatd, 1, 12, "rpc.ttdbserverd on %s OK"),
			   (char *)_dbref->envname);
	}
	auth_level = *result;
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc: auth_level = %d\n", auth_level);
	}
	switch (auth_level) {
	      case AUTH_NONE: case AUTH_UNIX:
		server->cl_auth = authunix_create_default();
		break;
#if defined(OPT_SECURE_RPC)
	      case AUTH_DES:
		struct hostent *hp;
		if (host2netname(servernetname, _dbref->envname, 0) &&
		    (hp = gethostbyname((char *)_dbref->envname))) {
			memcpy((caddr_t) &server_addr.sin_addr,
			       hp->h_addr,
			       hp->h_length);
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = 0;
		} else {
			server = 0;
		}
		// If gethostbyname was called we need to do endhostent
		// to free storage.  If gethostbyname was not called,
		// it doesn\'t hurt to call it.
		endhostent();
		break;
#endif /* OPT_SECURE_RPC */		
	      default:
		server = 0;
	}
	/* initialize transaction records buffer */
	_trecs = new _Tt_trans_list();
}

_Tt_db_desc::
~_Tt_db_desc()
{
	if (server) {
		if (server->cl_auth) {
			auth_destroy(server->cl_auth);
		}
		clnt_destroy(server);
	}
}

_Tt_dm_status _Tt_db_desc::
open()
{
	if (!server) {
		return (errcode = DM_OPEN_FAILED);
	}
		
	// Since, unknown to the dm user, we reuse db_descs, an
	// open may not really do anything. However, it\'s important
	// to at least reset the key id so read calls will not pick up
	// some old leftover position and read forward instead of
	// doing an isstart and reading by key. See bug 1048884.

	read_key_id = DM_KEY_UNDEFINED;

	if (num_opens > 0) {
		num_opens++;
		return (errcode = DM_OK);
	}

	int mode = ISMANULOCK + ISINOUT;
	if (table->variable_recp()) {
		mode += ISVARLEN;
	} else {
		mode += ISFIXLEN;
	}

	/* construct full path of database */
	char full_path[MAXPATHLEN];
	char *db_name = _dbname;
	sprintf(full_path, "%s%s%s", (char *) _dbref->relpath,
		"TT_DB/", db_name);
	if (DM_DEBUG(1)) {
		fprintf(stderr,"_Tt_db_desc::open - opening database %s . . .\n",
		       full_path);
	}

	/* do the open */
	if ((isfd = isopen(full_path, mode)) == -1) {
		switch (iserrno) {
		      case ENOENT:
			if (DM_DEBUG(5)) {
				fprintf(stderr,"Error: can't open database %s, file does not exist\n", full_path);
			}
			break;
		      case EACCES:
			if (DM_DEBUG(5)) {
				fprintf(stderr,"Error: can't open database %s, access permission denied\n", full_path);
			}
			break;
		      default:
			if (DM_DEBUG(5)) {
				fprintf(stderr,"Error: can't open database %s, errno = %d\n", full_path, iserrno);
			}
			break;
		}
		return (errcode = DM_OPEN_FAILED);
	} else {
		/* Check validity of database version number */
		if (iscntl(isfd, ISCNTL_APPLMAGIC_READ, magic.magicstring())
		    == -1) {
			isclose(isfd);
			return (errcode = DM_OPEN_FAILED);
		}
		if (magic.versionnumber() == INITIAL_VERSION_NO) {
			magic.setverno(THIS_VERSION_NO);
			if (iscntl(isfd, ISCNTL_APPLMAGIC_WRITE,
				   magic.magicstring()) == -1) {
				isclose(isfd);
				return (errcode = DM_OPEN_FAILED);
			}
		} else if (magic.versionnumber() != THIS_VERSION_NO) {
			isclose(isfd);
			return (errcode = DM_INVALID_VERSION_NUMBER);
		}
	}
	num_opens++;
	if (num_opens == 1) {
		_tt_dm->inc_num_isfds();
	}
	if (DM_DEBUG(1)) {
		fprintf(stderr,"_Tt_db_desc::open - succeeded with isfd = %d\n", isfd);
	}
	return (errcode = DM_OK);
}

/*
 * _Tt_db_desc::create - creates an empty NetISAM database as specified
 * for this database descriptor.  The database\'s magic string contains the
 * database version number (initially 0).  Returns one of the following code:
 *
 *	DM_OK  -  successful
 *	DM_DB_EXISTS  -  database is already existent
 *	DM_CREATE_FAILED  -  create failed.
 */

_Tt_dm_status _Tt_db_desc::
create()
{
	if (!server) {
		return (errcode = DM_CREATE_FAILED);
	}

	if (num_opens > 0) {
		ASSERT(isfd != -1, "no create, database is already opened");
		return (errcode = DM_OK);
	}
	ASSERT(table, "No table descriptor");

	/* setup the create mode */
	int mode = ISINOUT;
	int reclen;
	if (table->variable_recp()) {
		mode += ISVARLEN;
		isreclen = table->minreclen();	/* minimum len */
		reclen = ISMAXRECLEN;	/* maximum len */
	} else {
		mode += ISFIXLEN;
		reclen = table->minreclen();
	}

	/* construct the full path for the database */
	char full_path[MAXPATHLEN];
	char *db_name = _dbname;
	sprintf(full_path, "%s%s%s", (char *) _dbref->relpath,
		"TT_DB/", db_name);
	if (DM_DEBUG(1)) {
		fprintf(stderr,"DM:create - creating database %s . . .\n",
		       full_path);
	}

	/* check to see if the database is already created */
	if (((isfd = isopen(full_path, mode+ISMANULOCK)) == -1) &&
	    ((iserrno != ENOENT) && (iserrno != EFNAME))) {
		if (DM_DEBUG(5)) {
			fprintf(stderr,"_Tt_db_desc::create - open check failed, iserrno = %d\n", iserrno);
		}
		switch (iserrno) {
		      case ERPC:
			_tt_syslog(0, LOG_ERR,
				   catgets(_ttcatd, 1, 13,
					   "ERROR: rpc.ttdbserverd daemon on "
					   "%s is not installed [correctly]"),
				   (char *)_dbref->envname);
		      default:
			break;
		}
		return (errcode = DM_CREATE_FAILED);
	} else if (isfd != -1) {
		isclose(isfd);
		isfd = -1;
		if (DM_DEBUG(1)) {
			fprintf(stderr,
				"DM: create - database exists\n");
		}
		return (errcode = DM_OK); /* database already created */
	}
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::create - isopen errno = %d\n", iserrno);
	}

	/* create the database */
	_Tt_key_desc_list_cursor c(table->keys());
	c.next();
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::create - mode = 0x%X, reclen = %d, isreclen = %d\n",
		       mode, reclen, isreclen);
		c->print(stderr);
	}
	mode_t prev_mask = umask(0);	/* rwx for everyone  */
	if ((isfd = isbuild(full_path, reclen, c->key_desc(), mode+ISEXCLLOCK)) == -1) {
		if (DM_DEBUG(1)) {
			fprintf(stderr,"DM:create - isbuild iserrno = %d,"
				"returning DM_CREATE_FAILED\n",
				iserrno);
		}
		umask(prev_mask);
		return (errcode = DM_CREATE_FAILED);
	}
	umask(prev_mask);

	/* add secondary keys */
	if (!table->primarykeyp()) { 	/* table has only secondary keys */
		while (c.next()) {
			if (DM_DEBUG(5)) {
				fprintf(stderr,"_Tt_db_desc::create - adding key\n");
				c->print(stderr);
			}
			if (isaddindex(isfd, c->key_desc()) == -1) {
				if (DM_DEBUG(5)) {
					fprintf(stderr,"_Tt_db_desc::create - isaddindex errno = %d\n", iserrno);
				}
				iserase(full_path);
				return (errcode = DM_CREATE_FAILED);
			}
		}
	}

	/* Write the magic string with the initial version number of 0 to
	   signify that this database is just created. */
	magic.setverno(INITIAL_VERSION_NO);
	if (iscntl(isfd, ISCNTL_APPLMAGIC_WRITE, magic.magicstring()) == -1) {
		iserase(full_path);
		return (errcode = DM_CREATE_FAILED);
	}
	if (isclose(isfd) == -1) {
		return (errcode = DM_CREATE_FAILED);
	}
	isfd = -1;
	if (DM_DEBUG(1)) {
		fprintf(stderr,"DM:create - database %s created.\n",
		       full_path);
	}

	return (errcode = DM_OK);
}

/*
 *  _Tt_db_desc::read - reads the next record in this database that matches
 *  the specified key.  The input record \'rec\' does not need to contain the
 *  key\'s id, and a matching field (reading by physical order).  If successful,
 *  the input record \'rec\' is filled with the content of the matched record
 *  from the database.  One of the following codes is set:
 *
 *	DM_OK		  - successful
 *	DM_RECORD_LOCKED  - record is locked by another process.
 *	DM_NO_RECORD	  - no record in database matched key.
 *	DM_READ_FAILED    - the read attempt failed.
 */


_Tt_dm_status _Tt_db_desc::
read(_Tt_record_ptr rec)
{
	_Tt_key_desc_ptr kd;
	int isrc;

	rec->determine_key();
	if (!server) {
		return (errcode = DM_READ_FAILED);
	}
	isreclen = rec->rec_len;
	
	if (num_opens == 0) {
		ASSERT(isfd != -1, "can't read, database is not opened");
		return (errcode = DM_READ_FAILED);
	}
	if (DM_DEBUG(2)) {
		fprintf(stderr,"DM:read. Last key id = %s.\n"
			"Record before read:",
			_tt_enumname(read_key_id));
		rec->print(stderr);
	}
	ASSERT(rec->key_id != DM_KEY_UNDEFINED, "can't read, undefined key");
	ASSERT(rec->key_id == DM_KEY_NONE || rec->reclen>0,
	       "can't read, no key data given.");

	// If we are reading by a different index, or with a different key,
	// we must make an isstart call first to set the key.
	// The first time, read_key_id is DM_KEY_UNDEFINED which will never
	// match rec->key_id.
// following lines should be but cfront restrictions force the circumlocution
//      if (rec->key_id == read_key_id && saved_key==rec->extract_key(table))) {
	int isstart_needed = 0;
	if (rec->key_id != read_key_id) {
		isstart_needed = 1;
	} else if (saved_key!=rec->extract_key(table)) {
		isstart_needed = 1;
	}
	if (isstart_needed) {
		read_key_id = rec->key_id; /* save the key ID for iterator */
		kd = findkeydesc(rec->key_id);
		ASSERT(!kd.is_null(),"No key descriptor for rec->key_id");
		if (rec->key_id == DM_KEY_NONE) {
			/* read by physical order */
			isrc = isstart(isfd, kd->key_desc(), 0, NULL, ISFIRST);
		} else {
			/* read by key */
			isrc = isstart(isfd, kd->key_desc(), rec->key_len,
				       rec->bufadr(), ISEQUAL);
		}
		if (isrc==-1) {
			read_key_id = DM_KEY_UNDEFINED;
			if (iserrno == ENOREC) {
				errcode = DM_NO_RECORD;
			} else {
				errcode = DM_READ_FAILED;
			}
			if (DM_DEBUG(2)) {
				fprintf(stderr,
					"DM:isstart iserrno=%d. "
					"read returning %s\n",
					iserrno,
					_tt_enumname(errcode));
			}
			return errcode;
		}
	}

	/* setup the read mode */
	int mode;
	if (rec->key_id == DM_KEY_NONE) { /* read by record number */
		mode = ISEQUAL;
		isrecnum = rec->record_num();
	} else {		// read by index
		mode = ISNEXT;	
		/* save the specified key to compare after read. */
		saved_key = rec->extract_key(table);
	}

	/* do the read */
	if (isread(isfd, rec->bufadr(), mode) == -1) {
		switch (iserrno) {
		      case ELOCKED:
			errcode = DM_RECORD_LOCKED;
			break;
		      case EENDFILE:
		      case ENOREC:
			read_key_id = DM_KEY_UNDEFINED;
			errcode = DM_NO_RECORD;
			break;
		      case DM_ACCESS_DENIED:
			errcode = DM_ACCESS_DENIED;
			break;
		      default:
			read_key_id = DM_KEY_UNDEFINED;
			errcode = DM_READ_FAILED;
			break;
		}
		if (DM_DEBUG(2)) {
			fprintf(stderr,
				"DM:isread iserrno=%d. "
				"read returning %s\n",
				iserrno,
				_tt_enumname(errcode));
		}
		return errcode;
	}

	/* compare given key and key read to make sure they match */
	if (rec->key_id != DM_KEY_NONE) {
		_Tt_string retrieved_key = rec->extract_key(table);
		if (retrieved_key!=saved_key) {
			read_key_id = DM_KEY_UNDEFINED;
			errcode = DM_NO_RECORD;
			if (DM_DEBUG(2)) {
				fprintf(stderr,
					"DM:key read did not match given key."
					
					"read returning %s\n",
					_tt_enumname(errcode));
				fprintf(stderr,"saved key:");
				saved_key->print(stderr);
				fprintf(stderr,"\nretrieved key:");
				retrieved_key->print(stderr);
				fprintf(stderr,"\n");
			}
			return errcode;
		}
	}
		
	if (variable_recp()) {
		rec->set_reclength(isreclen);
	}
	rec->set_recnum(isrecnum);
	rec->setold();
	if (DM_DEBUG(2)) {
		fprintf(stderr,	"DM:read succeeded.");
		rec->print(stderr);
	}
	return (errcode = DM_OK);
}

/*
 *  _Tt_db_desc::readfirst - reads the first record in the database that
 *  has a key equal to or greater than
 *  the specified key. 
 *  the input record \'rec\' is filled with the content of the matched record
 *  from the database.  One of the following codes is set:
 *
 *	DM_OK		  - successful
 *	DM_RECORD_LOCKED  - record is locked by another process.
 *	DM_NO_RECORD	  - no record in database matched key (EOF).
 *	DM_READ_FAILED    - the read attempt failed.
 */
_Tt_dm_status _Tt_db_desc::
readfirst(_Tt_record_ptr rec)
{
	_Tt_key_desc_ptr kd;
	int isrc;

	rec->determine_key();
	if (!server) {
		return (errcode = DM_READ_FAILED);
	}
	isreclen = rec->rec_len;
	if (num_opens == 0) {
		ASSERT(isfd != -1, "can't read, database is not opened");
		return (errcode = DM_READ_FAILED);
	}
	if (DM_DEBUG(1)) {
		fprintf(stderr,"DM:readfirst. Record before read:\n",
			_tt_enumname(read_key_id));
		rec->print(stderr);
	}
	ASSERT(rec->key_id != DM_KEY_UNDEFINED, "can't read, undefined key");
	ASSERT(rec->key_id == DM_KEY_NONE || rec->reclen>0,
	       "can't read, no key data given.");

	kd = findkeydesc(rec->key_id);
	ASSERT(!kd.is_null(),"No key descriptor for rec->key_id");
	if (rec->key_id == DM_KEY_NONE) {
		/* read by physical order */
		isrc = isstart(isfd, kd->key_desc(), 0, NULL, ISFIRST);
	} else {
		/* read by key */
		isrc = isstart(isfd, kd->key_desc(), rec->key_len,
			       rec->bufadr(), ISGTEQ);
	}
	if (isrc==-1) {
		if (iserrno == ENOREC) {
			errcode = DM_NO_RECORD;
		} else {
			errcode = DM_READ_FAILED;
		}
		if (DM_DEBUG(1)) {
			fprintf(stderr,
				"DM:isstart iserrno=%d. "
				"readfirst returning %s\n",
				iserrno,
				_tt_enumname(errcode));
		}
		return errcode;
	}


	/* setup the read mode */
	int mode;
	if (rec->key_id == DM_KEY_NONE) { /* read by record number */
		mode = ISEQUAL;
		isrecnum = rec->record_num();
	} else {		// read by index
		mode = ISNEXT;	
	}

	/* do the read */
	if (isread(isfd, rec->bufadr(), mode) == -1) {
		switch (iserrno) {
		      case ELOCKED:
			errcode = DM_RECORD_LOCKED;
			break;
		      case EENDFILE:
		      case ENOREC:
			read_key_id = DM_KEY_UNDEFINED;
			errcode = DM_NO_RECORD;
			break;
		      case DM_ACCESS_DENIED:
			errcode = DM_ACCESS_DENIED;
			break;
		      default:
			read_key_id = DM_KEY_UNDEFINED;
			errcode = DM_READ_FAILED;
			break;
		}
		if (DM_DEBUG(1)) {
			fprintf(stderr,
				"DM:isread iserrno=%d. "
				"readfirst returning %s\n",
				iserrno,
				_tt_enumname(errcode));
		}
		return errcode;
	}
	if (variable_recp()) {
		rec->set_reclength(isreclen);
	}
	rec->set_recnum(isrecnum);
	rec->setold();
	if (DM_DEBUG(1)) {
		fprintf(stderr,	"DM:readfirst succeeded, record is:\n");
		rec->print(stderr);
	}
	return (errcode = DM_OK);
}

/*
 *  _Tt_db_desc::readnext - reads the next record in this database
 *  must follow an initial readfirst() to set up indexing, etc.
 *
 *	DM_OK		  - successful
 *	DM_RECORD_LOCKED  - record is locked by another process.
 *	DM_NO_RECORD	  - no next record in database (i.e. EOF)
 *	DM_READ_FAILED    - the read attempt failed.
 */
_Tt_dm_status _Tt_db_desc::
readnext(_Tt_record_ptr rec)
{
	_Tt_key_desc_ptr kd;

	if (!server) {
		return (errcode = DM_READ_FAILED);
	}
	isreclen = rec->rec_len;
	
	if (num_opens == 0) {
		ASSERT(isfd != -1, "can't read, database is not opened");
		return (errcode = DM_READ_FAILED);
	}
	if (DM_DEBUG(1)) {
		fprintf(stderr,"DM:readnext. \n"
			"Record before read:",
			_tt_enumname(read_key_id));
		rec->print(stderr);
	}
	ASSERT(rec->key_id != DM_KEY_UNDEFINED, "can't read, undefined key");
	int mode = ISNEXT;	
	if (isread(isfd, rec->bufadr(), mode) == -1) {
		switch (iserrno) {
		      case ELOCKED:
			errcode = DM_RECORD_LOCKED;
			break;
		      case EENDFILE:
		      case ENOREC:
			read_key_id = DM_KEY_UNDEFINED;
			errcode = DM_NO_RECORD;
			break;
		      case DM_ACCESS_DENIED:
			errcode = DM_ACCESS_DENIED;
			break;
		      default:
			read_key_id = DM_KEY_UNDEFINED;
			errcode = DM_READ_FAILED;
			break;
		}
		if (DM_DEBUG(1)) {
			fprintf(stderr,
				"DM:isread iserrno=%d. "
				"readnext returning %s\n",
				iserrno,
				_tt_enumname(errcode));
		}
		return errcode;
	}
	if (variable_recp()) {
		rec->set_reclength(isreclen);
	}
	rec->set_recnum(isrecnum);
	rec->setold();
	if (DM_DEBUG(1)) {
		fprintf(stderr,	"DM:readnext succeeded, record is:\n");
		rec->print(stderr);
	}
	return (errcode = DM_OK);
}

/*
 *  _Tt_db_desc::test_and_set - test to see if there\'s a record that matches
 *  the specified record.  If so, returns the matched record and DM_RECORD_SET.
 *  Otherwise, writes the specified record and returns DM_OK.  If there\'s
 *  any error, returns DM_ERROR.
 */

_Tt_dm_status _Tt_db_desc::
test_and_set(_Tt_record_ptr rec)
{
	if (!server) {
		return (errcode = DM_ERROR);
	}
	rec->determine_key();
	create_auth();
	_Tt_test_and_set_args args;
	args.isfd = isfd;
	args.key = findkeydesc(rec->key_id)->key_desc();
	args.key_len = rec->key_len;
	args.rec.rec_len = rec->rec_len;
	args.rec.rec_val = rec->bufadr();
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::test_and_set - key_len = %d, rec_len = %d\n",
		       args.key_len, args.rec.rec_len);
	}
	_Tt_test_and_set_results *result = _tt_test_and_set_1(&args, server);
	reset_read();	/* reset next read */
	if (!result) {
		iserrno = ERPC;
		return (errcode = DM_ERROR);
	}
	isreclen = result->rec.rec_len;
	isrecnum = result->isrecnum;
	iserrno = result->isresult.iserrno;
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::test_and_set - result = %d, rec_len = %d, rec_num = %d, iserrno = %d\n",
		       result->isresult.result, isreclen, isrecnum, iserrno);
	}
	switch (result->isresult.result) {
	      case -1:
		if (iserrno >= DM_ERROR) {
			return (errcode = (_Tt_dm_status)iserrno);
		} else {
			return (errcode = DM_ERROR);
		}
	      case 0:
		/* record exist, returns it */
		memcpy(rec->bufadr(), result->rec.rec_val,
		       result->rec.rec_len);
		return (errcode = DM_RECORD_SET);
	      case 1:	/* record not exist, written */
		return (errcode = DM_OK);
	}

	return (errcode = DM_ERROR);
}

/*
 *  _Tt_db_desc::write - writes the specified record \'rec\' to this database.
 *  The record must be specified as NEW (default in constructor) or OLD
 *  (set by read_next).  Old records are rewritten (updated).  Set one of the
 *  following codes:
 *
 *	DM_OK  -  successful
 *	DM_WRITE_FAILED - the write failed
 */

_Tt_dm_status _Tt_db_desc::
write(_Tt_record_ptr rec)
{
	int t;
	
	if (!server) {
		return (errcode = DM_WRITE_FAILED);
	}

	if (num_opens == 0) {
		ASSERT(isfd != -1, "can't write, database is not opened");
		return (errcode = DM_WRITE_FAILED);
	}
	rec->determine_key();
	isreclen = rec->reclength();
	if (DM_DEBUG(2)) {
		fprintf(stderr,"DM:write");
		rec->print(stderr);
	}
	if (rec->newrecp()) {
		t = iswrite(isfd, rec->bufadr());
	} else {
		t = isrewrec(isfd, rec->record_num(),rec->bufadr());
	}
	if (t==-1) {
		switch (iserrno) {
		      case ELOCKED:
			errcode = DM_RECORD_LOCKED;
			break;
		      default:
			if (iserrno >= DM_ERROR) {
				errcode = (_Tt_dm_status)iserrno;
			} else {
				errcode = DM_WRITE_FAILED;
			}
			break;
		}
		if (DM_DEBUG(2)) {
			fprintf(stderr,
				"DM: iswrite/isrewrec failed, iserrno %d\n",
				iserrno);
		}
	} else {
		errcode = DM_OK;
	}
	if (DM_DEBUG(2)) {
		fprintf(stderr,"DM: write returned %s\n",
			_tt_enumname(errcode));
	}
	return(errcode);
}

/*
 * _Tt_db_desc::remove - delete the record specified by recnum from this
 * database.  If succesful, returns DM_OK.  Otherwise, set errcode to
 * DM_DELETE_FAILED.
 */

_Tt_dm_status _Tt_db_desc::
remove(_Tt_record_ptr rec) 
{
	if (!server || num_opens == 0) {
		return (errcode = DM_DELETE_FAILED);
	}
	/* Must provide key for access control */
	isreclen = OID_KEY_LENGTH;
	if (DM_DEBUG(5)) {
		fprintf(stderr,"_Tt_db_desc::remove - isreclen = %d\n", isreclen);
	}
	if (isdelrec(isfd, rec->record_num(), rec->bufadr()) == -1) {
		return (errcode = DM_DELETE_FAILED);
	} else {
		return (errcode = DM_OK);
	}
}

/*
 *  _Tt_db_desc::close() - close this database.  If successful, returns DM_OK.
 *  Otherwise, set errcode to DM_CLOSE_FAILED.
 */

_Tt_dm_status _Tt_db_desc::
close() {
	errcode = DM_OK;
	iserrno = 0;

	if (DM_DEBUG(1)) {
		fprintf(stderr,
			"_Tt_db_desc::close isfd = %d, num_opens = %d\n",
			isfd, num_opens);
	}

	if (!server) {
		errcode = DM_CLOSE_FAILED;
	} else 	if (num_opens == 0) {
		errcode = DM_CLOSE_FAILED;
	}

        num_opens--;
	if (num_opens == 0) {
		if (isclose(isfd) == -1) {
			errcode = DM_CLOSE_FAILED;
		} else {
			isfd = -1;
			_tt_dm->delete_dbdesc(this);
			_tt_dm->dec_num_isfds();
		}
	}
	if (DM_DEBUG(1)) {
		fprintf(stderr,
			"_Tt_db_desc::close returning %s.  "
			"iserrno = %d\n",
			_tt_enumname(errcode), iserrno);
	}
	return errcode;
}

_Tt_dm_status _Tt_db_desc::
setuid(_Tt_oid_ptr oid, uid_t uid)
{
	_Tt_isam_results *res;
	_Tt_oidaccess_args args;

	args.isfd = isfd;
	args.key.key_len = OID_KEY_LENGTH;
	args.key.key_val = oid->key()->content();
	if (uid == (uid_t)-1) {
		args.value = (short)_tt_dm->user_id;
	} else {
		args.value = (short)uid;
	}
	res = _tt_setoiduser_1(&args, server);
	errcode = (_Tt_dm_status)res->result;
	return errcode;
}

_Tt_dm_status _Tt_db_desc::
setgroup(_Tt_oid_ptr oid, gid_t group)
{
	_Tt_isam_results *res;
	_Tt_oidaccess_args args;

	args.isfd = isfd;
	args.key.key_len = OID_KEY_LENGTH;
	args.key.key_val = oid->key()->content();
	args.value = (short)group;
	res = _tt_setoidgroup_1(&args, server);
	errcode = (_Tt_dm_status)res->result;
	return errcode;
}

_Tt_dm_status _Tt_db_desc::
setmode(_Tt_oid_ptr oid, mode_t mode)
{
	_Tt_isam_results *res;
	_Tt_oidaccess_args args;

	args.isfd = isfd;
	args.key.key_len = OID_KEY_LENGTH;
	args.key.key_val = oid->key()->content();
	args.value = (short)mode;
	res = _tt_setoidmode_1(&args, server);
	errcode = (_Tt_dm_status)res->result;
	return errcode;
}

_Tt_dm_status _Tt_db_desc::
getaccess(_Tt_oid_ptr oid, uid_t &uid, gid_t &group, mode_t &mode)
{
	_Tt_oidaccess_results *res;
	_Tt_oidaccess_args args;

	args.isfd = isfd;
	args.key.key_len = OID_KEY_LENGTH;
	args.key.key_val = oid->key()->content();
	args.value = 0;
	res = _tt_getoidaccess_1(&args, server);
	uid = res->uid;
	group = res->group;
	mode = res->mode;
	errcode = (_Tt_dm_status)res->result;
	return errcode;
}

#ifdef UNUSED
// XXX: Much of this old lib/dm code is apparently not used any more.
// Until we make an organized effort to get rid of it, I will just
// ifdef out things as I find them.  RFM 7/16/93
_Tt_dm_status _Tt_db_desc::
readspec(_Tt_oid_ptr oid, _Tt_prop_array &props)
{
	_Tt_spec_props *res;
	_Tt_spec_props args;

	args.isfd = isfd;
	args.oidkey.oidkey_len = OID_KEY_LENGTH;
	args.oidkey.oidkey_val = oid->key()->content();
	args.props.props_len = 0;
	args.props.props_val = 0;
	res = _tt_readspec_1(&args, server);
	props = res->props;
	errcode = (_Tt_dm_status)res->result;
	return errcode;
}

_Tt_dm_status _Tt_db_desc::
writespec(_Tt_oid_ptr oid, _Tt_prop_array props)
{
	_Tt_isam_results *res;
	_Tt_spec_props args;

	args.isfd = isfd;
	args.oidkey.oidkey_len = OID_KEY_LENGTH;
	args.oidkey.oidkey_val = oid->key()->content();
	args.props.props_len = props.props_len;
	args.props.props_val = props.props_val;
	res = _tt_writespec_1(&args, server);
	errcode = (_Tt_dm_status)res->result;
	return errcode;
}

_Tt_dm_status _Tt_db_desc::
addsession(_Tt_oid_ptr oid, _Tt_string session)
{
	_Tt_isam_results *res;
	_Tt_session_args args;

	args.isfd = isfd;
	args.key = findkeydesc(DM_KEY_OID_PROP)->key_desc();
	args.key_len = OID_KEY_LENGTH + MAX_PROP_LENGTH;
	args.oidkey.oidkey_len = OID_KEY_LENGTH;
	args.oidkey.oidkey_val = oid->key()->content();
	args.session.session_len = session.len();
	args.session.session_val = (char *)session;
	res = _tt_addsession_1(&args, server);
	errcode = (_Tt_dm_status)res->result;
	return errcode;
}

_Tt_dm_status _Tt_db_desc::
delsession(_Tt_oid_ptr oid, _Tt_string session)
{
	_Tt_isam_results *res;
	_Tt_session_args args;

	args.isfd = isfd;
	args.key = findkeydesc(DM_KEY_OID_PROP)->key_desc();
	args.key_len = OID_KEY_LENGTH + MAX_PROP_LENGTH;
	args.oidkey.oidkey_len = OID_KEY_LENGTH;
	args.oidkey.oidkey_val = oid->key()->content();
	args.session.session_len = session.len();
	args.session.session_val = (char *)session;
	res = _tt_delsession_1(&args, server);
	errcode = (_Tt_dm_status)res->result;
	return errcode;
}

#endif /* UNUSED */


_Tt_dm_status _Tt_db_desc::
gettype(_Tt_oid_ptr oid, _Tt_string typeprop, _Tt_string &typename)
{
	_Tt_spec_props *res;
	_Tt_spec_props args;
	_Tt_prop props, *propp;

	args.isfd = isfd;
	args.key = findkeydesc(DM_KEY_OID_PROP)->key_desc();
	args.key_len = OID_KEY_LENGTH + MAX_PROP_LENGTH;
	args.oidkey.oidkey_len = OID_KEY_LENGTH;
	args.oidkey.oidkey_val = oid->key()->content();
	args.props.props_len = 1;
	args.props.props_val = &props;
	props.propname.propname_val = (char *)typeprop;
	props.propname.propname_len = typeprop.len();
	props.recnum = 0;
	props.value.value_len = 0;
	props.value.value_val = 0;
	res = _tt_gettype_1(&args, server);
	if (res->result == DM_OK) {
		propp = &(res->props.props_val[0]);
		typename.set((unsigned char *)propp->value.value_val,
			     propp->value.value_len);
	}
	errcode = (_Tt_dm_status)res->result;
	xdr_free((xdrproc_t)xdr__Tt_spec_props, (char *)res);
	return errcode;
}

_Tt_dbtable_id _Tt_db_desc::
tableid() const
{
	return table->tableid();
}

void _Tt_db_desc::
settable(_Tt_table_desc_ptr db_table)
{
	table = db_table;
}

_Tt_dbref_ptr _Tt_db_desc::
dbref()
{
	return _dbref;
}

_Tt_name_space _Tt_db_desc::
namespace() const
{
	return _tt_dm->namespace;
}

_Tt_string _Tt_db_desc::
envname()
{
	return _dbref->envname;
}

_Tt_string _Tt_db_desc::
path()
{
	return _dbref->relpath;
}

_Tt_string _Tt_db_desc::
dbname()
{
	return _dbname;
}

_Tt_key_desc_ptr _Tt_db_desc::
findkeydesc(_Tt_key_id key_id) const
{
	if (table.is_null()) {
		return 0;
	} else {
		return table->findkeydesc(key_id);
	}
}

int _Tt_db_desc::
variable_recp() const
{
	return table->variable_recp();
}


