//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 * Tool Talk Database Manager (DM) - dm_mfs.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * Definition of class _Tt_mfs which implements a linked element containing
 * a mount table (/etc/mtab) entry.
 *
 */

#include "tt_options.h"
#include <stdio.h>
#include <dm/dm_mfs.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#if defined(ultrix)
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mount.h>
#endif

_Tt_mfs::
_Tt_mfs(const MNTENT *e)
{
#ifdef _AIX
	_mntentry = (MNTENT *)malloc(e->vmt_length);
	memcpy(_mntentry, e, e->vmt_length);
#elif defined(ultrix)
	_mntentry = (MNTENT *)malloc(sizeof(MNTENT));
	memcpy(_mntentry, e, sizeof(MNTENT));
#else
	_mntentry = (MNTENT *)malloc(sizeof(MNTENT));
	_mntentry->MNT_FSNAME = (char *)malloc(strlen(e->MNT_FSNAME)+1);
	strcpy(_mntentry->MNT_FSNAME, e->MNT_FSNAME);
	_mntentry->MNT_PT = (char *)malloc(strlen(e->MNT_PT)+1);
	strcpy(_mntentry->MNT_PT, e->MNT_PT);
	_mntentry->MNT_TYPE = (char *)malloc(strlen(e->MNT_TYPE)+1);
	strcpy(_mntentry->MNT_TYPE, e->MNT_TYPE);
	_mntentry->MNT_OPTS = (char *)malloc(strlen(e->MNT_OPTS)+1);
	strcpy(_mntentry->MNT_OPTS, e->MNT_OPTS);
#if !defined(OPT_SVR4_GETMNTENT)
	_mntentry->mnt_freq = e->mnt_freq;
	_mntentry->mnt_passno = e->mnt_passno;
#endif /* SVR4 */
#endif /* _AIX */
}

_Tt_mfs::
~_Tt_mfs()
{
	free((MALLOCTYPE *)_mntentry);
}

MNTENT *_Tt_mfs::
mntentry() const
{
	return _mntentry;
}

void _Tt_mfs::
print(FILE *fs) const
{
	fprintf(fs, "filesystem name: %s\n", mount_fsname(_mntentry));
	fprintf(fs, "mount point:     %s\n", mount_point(_mntentry));
	fprintf(fs, "filesystem type: %s\n", mount_type(_mntentry));
}
