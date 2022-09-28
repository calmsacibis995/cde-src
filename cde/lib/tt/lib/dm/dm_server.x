/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
#define	MAXPATHLEN	1024
#define	MAXHOSTNAMELEN  64
#define ISMAXRECLEN	8192		     /* Maximum number of bytes in record */

#define NPARTS		8		     /* max number of key parts */

struct keypart {
    short 	kp_start;		     /* starting byte of key part */
    short	kp_leng;		     /* length in bytes */
    short	kp_type;		     /* type of key part */
};

struct keydesc {
    short 	k_flags;		     /* flags */
    short	k_nparts;		     /* number of parts in key */
    struct keypart k_part[NPARTS];	     /* each part */
};

struct _Tt_isam_results {
	int	result;
	int	iserrno;
};

struct _Tt_isaddindex_args {
	int	isfd;
	keydesc	*key;
};

struct _Tt_isbuild_args {
	string	path<MAXPATHLEN>;
	int	reclen;
	keydesc	*key;
	int	mode;
	int	isreclen;
};

struct _Tt_iscntl_args {
	int	isfd;
	int	func;
	char	arg<ISAPPLMAGICLEN>;
};

struct _Tt_iscntl_results {
	int	result;
	int	iserrno;
	char	arg<ISAPPLMAGICLEN>;
};

struct _Tt_isdelrec_args {
	int	isfd;
	long	recnum;
	char	rec<ISMAXRECLEN>;
};

struct _Tt_isopen_args {
	string	path<MAXPATHLEN>;
	int	mode;
};

struct _Tt_isread_args {
	int	isfd;
	char	rec<ISMAXRECLEN>;
	int	mode;
	long	isrecnum;
};

struct _Tt_isread_results {
	_Tt_isam_results	isresult;
	char			rec<ISMAXRECLEN>;
	int			isreclen;
	long			isrecnum;
};

struct _Tt_isrewrec_args {
	int	isfd;
	long	recnum;
	char	rec<ISMAXRECLEN>;
};

struct _Tt_isstart_args {
	int	isfd;
	keydesc	*key;
	int	key_len;
	char	rec<ISMAXRECLEN>;
	int	mode;
};

struct _Tt_iswrite_args {
	int	isfd;
	char	rec<ISMAXRECLEN>;
};
	
struct _Tt_test_and_set_args {
	int	isfd;
	keydesc *key;
	int	key_len;
	char	rec<ISMAXRECLEN>;
};

struct _Tt_test_and_set_results {
	_Tt_isam_results isresult;
	char	rec<ISMAXRECLEN>;
	int	isreclen;
	long	isrecnum;
};

struct _Tt_trans_record {
	int	newp;
	long	recnum;
	char	rec<ISMAXRECLEN>;
	_Tt_trans_record_list next;
};

struct _Tt_transaction_args {
	int	isfd;
	_Tt_trans_record_list recs;
};

struct _Tt_oidaccess_args {
	int	isfd;
	char	key<OID_KEY_LENGTH>;
	short	value;
};

struct _Tt_oidaccess_results {
	uid_t uid;
	gid_t group;
	mode_t mode;
	int result;
	int iserrno;
};

struct _Tt_prop {
	char	propname<>;
	long	recnum;
	char	value<ISMAXRECLEN>;
};

struct _Tt_spec_props {
	int	isfd;
	keydesc *key;
	int key_len;
	char	oidkey<OID_KEY_LENGTH>;
	_Tt_prop props<>;
	int result;
	int iserrno;
};

struct _Tt_session_args {
	int	isfd;
	keydesc	*key;
	int key_len;
	char	oidkey<OID_KEY_LENGTH>;
	char	session<>;
};

program TT_DBSERVER_PROG {
    version TT_DBSERVER_VERS {
	int			_TT_MIN_AUTH_LEVEL(string) = 1;
	_Tt_isam_results 	_TT_ISADDINDEX(_Tt_isaddindex_args) = 2;
	_Tt_isam_results 	_TT_ISBUILD(_Tt_isbuild_args) = 3;
	_Tt_isam_results 	_TT_ISCLOSE(int) = 4;
	_Tt_iscntl_results 	_TT_ISCNTL(_Tt_iscntl_args) = 5;
	_Tt_isam_results 	_TT_ISDELREC(_Tt_isdelrec_args) = 6;
	_Tt_isam_results 	_TT_ISERASE(string) = 7;
	_Tt_isam_results 	_TT_ISOPEN(_Tt_isopen_args) = 8;
	_Tt_isread_results	_TT_ISREAD(_Tt_isread_args) = 9;
	_Tt_isam_results 	_TT_ISREWREC(_Tt_isrewrec_args) = 10;
	_Tt_isam_results 	_TT_ISSTART(_Tt_isstart_args) = 11;
	_Tt_isam_results 	_TT_ISWRITE(_Tt_iswrite_args) = 12;
	_Tt_test_and_set_results _TT_TEST_AND_SET(_Tt_test_and_set_args) = 13;
	_Tt_isam_results	_TT_TRANSACTION(_Tt_transaction_args) = 14;
	string			_TT_MFS(string) = 15;
	_Tt_oidaccess_results	_TT_GETOIDACCESS(_Tt_oidaccess_args) = 16;
	_Tt_isam_results	_TT_SETOIDUSER(_Tt_oidaccess_args) = 17;
	_Tt_isam_results	_TT_SETOIDGROUP(_Tt_oidaccess_args) = 18;
	_Tt_isam_results	_TT_SETOIDMODE(_Tt_oidaccess_args) = 19;
	_Tt_spec_props		_TT_READSPEC(_Tt_spec_props) = 20;
	_Tt_isam_results	_TT_WRITESPEC(_Tt_spec_props) = 21;
	_Tt_isam_results	_TT_ADDSESSION(_Tt_session_args) = 22;
	_Tt_isam_results	_TT_DELSESSION(_Tt_session_args) = 23;
	_Tt_spec_props		_TT_GETTYPE(_Tt_spec_props) = 24;

    } = 1;
} = 100083;
