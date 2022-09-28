/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*      mount.h     1.8     88/02/08     */

/*
 * Copyright (c) 1984 Sun Microsystems, Inc.
 */

#ifndef _TT_DM_RPC_MOUNT_H
#define _TT_DM_RPC_MOUNT_H

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include "tt_options.h"
#if defined(OPT_BUG_SUNOS_4) && defined(__GNUG__)
#	include <stdlib.h>
#endif
#include <rpc/rpc.h>
#if defined(ultrix)
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <rpc/types.h>
#define bool_t int
#endif

#if defined(sgi)
#include <sys/fs/nfs.h>
#else
#include <nfs/nfs.h>
#endif

#define MOUNTPROG 100005
#define MOUNTPROC_NULL 0
#define MOUNTPROC_MNT 1
#define MOUNTPROC_DUMP 2
#define MOUNTPROC_UMNT 3
#define MOUNTPROC_UMNTALL 4
#define MOUNTPROC_EXPORT 5
#define MOUNTPROC_EXPORTALL 6
#define MOUNTVERS_ORIG 1
#define MOUNTVERS 1

#define MNTPATHLEN 1024
#define MNTNAMLEN 255

#ifndef svc_getcaller
#define svc_getcaller(x) (&(x)->xp_raddr)
#endif

typedef struct mount_tt *mounts_list;

bool_t xdr_mounts_list(XDR*, mounts_list*);

struct mount_tt {               /* what is mounted */
	char 		*ml_name;
	char 		*ml_path;
	mounts_list	ml_nxt;
};

bool_t xdr_fhandle_t(XDR*, fhandle_t);
bool_t xdr_fhstatus(XDR*, struct fhstatus*);

struct fhstatus {
	int fhs_status;
	fhandle_t fhs_fh;
};

typedef struct groups *groups_list;
bool_t xdr_groups_list(XDR*, groups_list*);

struct groups {
	char		*g_name;
	groups_list	g_next;
};

/*
 * List of exported directories
 * An export entry with ex_groups
 * NULL indicates an entry which is exported to the world.
 */

typedef struct exports *exports_list;
bool_t xdr_exports_list(XDR*, exports_list*);
/* bool_t xdr_dev_t(XDR*, dev_t); */

struct exports {
/*	dev_t		ex_dev;		dev of directory */
	char		*ex_name;	/* name of directory */
	groups_list	ex_groups;	/* groups allowed to mount this entry */
	exports_list	ex_next;
/*	short		ex_rootmap;	 id to map root requests to */
/*	short		ex_flags;	 bits to mask off file mode */
};

#endif /* _TT_DM_RPC_MOUNT_H */
