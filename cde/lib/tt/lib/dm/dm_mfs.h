/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * Tool Talk Database Manager (DM) - dm_mfs.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Declaration of class _Tt_mfs which implements a linked element
 * containing a mount table (/etc/mtab) entry.
 *
 */

#ifndef  _TT_DM_MFS_H
#define  _TT_DM_MFS_H

#include <stdio.h>
#include "tt_options.h"

#if defined(OPT_SVR4_GETMNTENT)
#	include <sys/mntent.h>
#	include <sys/mnttab.h>
#	define MOUNTED		MNTTAB
#	define MNTENT		mnttab
#	define MNT_FSNAME	mnt_special
#	define MNT_PT		mnt_mountp
#	define MNT_TYPE		mnt_fstype
#	define MNT_OPTS		mnt_mntopts
#	define setmntent	fopen
#	define endmntent	fclose
#	define is_mount_local(x)	(strcmp((x)->mnt_fstype,MNTTYPE_UFS)==0)
#	define is_mount_readonly(x)	(strstr(x->mnt_mntopts, "ro") != 0)
#	define is_mount_nfs(x)		(strcmp((x)->mnt_fstype, "nfs") == 0)
#	define mount_fsname(x)		((x)->mnt_special)
#	define mount_nfs_fsname(x)	(strchr((x)->mnt_special, ':')+1)
#	define mount_options(x)		((x)->mnt_mntopts)
#	define mount_point(x)		((x)->mnt_mountp)
#	define mount_type(x)		((x)->mnt_fstype)
#	define set_mount_rhost(x, rhost) {		\
		int len = 0;				\
		while ((x)->mnt_special[len] != ':') {	\
			len++;				\
		}					\
		memcpy(rhost, (x)->mnt_special, len);	\
		(rhost)[len] = '\0';			\
	}
#elif defined(_AIX)
extern "C" int mntctl(int,int,char *);
#	include <sys/vmount.h>
#	define MNTENT		vmount
#	define setmntent(x,y)	((FILE *) 1)
#	define endmntent(x)
#	define is_mount_local(x)	((x)->vmt_gfstype == MNT_JFS)
#	define is_mount_readonly(x)	((x)->vmt_flags & MNT_READONLY)
#	define is_mount_nfs(x)		((x)->vmt_gfstype == MNT_NFS)
#	define mount_fsname(x)		\
		((char *) (x) + (x)->vmt_data[VMT_OBJECT].vmt_off)
#	define mount_nfs_fsname(x)	mount_fsname(x)
#	define mount_options(x)		\
			((char *)(x) + (x)->vmt_data[VMT_ARGS].vmt_off)
#	define mount_point(x)		\
			((char *)(x) + (x)->vmt_data[VMT_STUB].vmt_off)
#	define mount_type(x)	(((x)->vmt_gfstype == MNT_JFS) ? "jfs" \
				: (((x)->vmt_gfstype == MNT_NFS) ? "nfs" \
				 : (((x)->vmt_gfstype == MNT_CDROM) ? "cdrom" \
				  : (((x)->vmt_gfstype == MNT_AIX) ? "oaix" \
				   : "unknown"))))
#	define set_mount_rhost(x, rhost)\
		memcpy(rhost, (char *)(x) + (x)->vmt_data[VMT_HOST].vmt_off, \
				(x)->vmt_data[VMT_HOST].vmt_size);
#elif defined(ultrix)
#       include <sys/param.h>
#       include <sys/types.h>
#       include <sys/mount.h>
#       define MOUNTED "/etc/mtab"
#       define KERNEL
#       include </usr/sys/h/fs_types.h>
#       undef KERNEL
        extern "C"
        {
            extern int getmnt(int*, struct fs_data*, int, int, char*);
	    extern char *realpath(char *,char*);
	  }
#	define MNTENT		fs_data
#	define setmntent(x,y)	((FILE *) 1)
#	define endmntent(x)
#	define is_mount_local(x)	((x)->fd_fstype != GT_NFS)
#	define is_mount_readonly(x)	((x)->fd_flags & M_RONLY)
#	define is_mount_nfs(x)		((x)->fd_fstype == GT_NFS)
#	define mount_fsname(x)		((x)->fd_devname)
#	define mount_nfs_fsname(x)	mount_fsname(x)
#	define mount_options(x)		("")
#	define mount_point(x)		((x)->fd_path)
#	define mount_type(x)	(((x)->fd_fstype == GT_ULTRIX) ? "ufs" \
				 : (((x)->fd_fstype == GT_NFS) ? "nfs" \
			          : "unknown"))
#	define set_mount_rhost(x, rhost) {		\
		int len = 0;				\
		while ((x)->fd_devname[len] != ':') {	\
			len++;				\
		}					\
		memcpy(rhost, (x)->fd_devname, len);	\
		(rhost)[len] = '\0';			\
	}
#else
// SGI's <mntent.h> is unguarded.
#	if defined(sgi)
#		ifndef __MNTENT_H__
#			define __MNTENT_H__
#			include <mntent.h>
#		endif
#		define is_mount_local(x) (strcmp((x)->mnt_type, MNTTYPE_EFS)==0)
#	elif defined(hpux)
		extern "C" { extern char *realpath(char *, char *); }
#		include <mntent.h>
#		define is_mount_local(x) (strcmp((x)->mnt_type, MNTTYPE_HFS) == 0)
#	else
#		include <mntent.h>
#		define is_mount_local(x) (strcmp((x)->mnt_type, MNTTYPE_42)==0)
#	endif
#	define MNTENT		mntent
#	define MNT_FSNAME	mnt_fsname
#	define MNT_PT		mnt_dir
#	define MNT_TYPE		mnt_type
#	define MNT_OPTS		mnt_opts
#	define is_mount_readonly(x)	hasmntopt(x, "ro")
#	define is_mount_nfs(x)		(strcmp((x)->mnt_type, "nfs") == 0)
#	define mount_fsname(x)		((x)->mnt_fsname)
//#	define mount_nfs_fsname(x)	(strchr((x)->mnt_fsname,":")+1)
#       define mount_nfs_fsname(x)      mount_fsname(x)
#	define mount_options(x)		((x)->mnt_opts)
#	define mount_point(x)		((x)->mnt_dir)
#	define mount_type(x)		((x)->mnt_type)
#	define set_mount_rhost(x, rhost) {		\
		int len = 0;				\
		while ((x)->mnt_fsname[len] != ':') {	\
			len++;				\
		}					\
		memcpy(rhost, (x)->mnt_fsname, len);	\
		(rhost)[len] = '\0';			\
	}
#endif /* SVR4 */

#include <util/tt_object.h>
#include <util/tt_list.h>

class _Tt_mfs : public _Tt_object {
      public:
	_Tt_mfs() {}
	_Tt_mfs(const MNTENT *e);
	virtual ~_Tt_mfs();
	MNTENT		*mntentry() const;
	void		print(FILE *fs = stdout) const;
      private:
	MNTENT		*_mntentry;
};

declare_list_of(_Tt_mfs)

#endif /* _TT_DM_MFS_H */
