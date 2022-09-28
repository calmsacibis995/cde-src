//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *  Tool Talk Database Manager - dm_rpc_mount.cc
 *
 *  Copyright (c) 1989 Sun Microsystems, Inc.
 *
 *  Implementation of the RPC mount XDR functions.
 */

#include <dm/dm_rpc_mount.h>
#include <stdio.h>

bool_t
xdr_fhandle_t(XDR *xdrs, fhandle_t *objp)
{
	return xdr_opaque(xdrs, (char *)objp, NFS_FHSIZE);
}

bool_t
xdr_fhstatus(XDR *xdrs,	struct fhstatus *objp)
{
	if (!xdr_int(xdrs, &objp->fhs_status)) {
		return (FALSE);
	}
	switch (objp->fhs_status) {
	      case 0:
		if (xdr_fhandle_t(xdrs, &objp->fhs_fh)) {
			return (FALSE);
		}
		break;
	      default:
		break;
	}
	return (TRUE);
}

bool_t
xdr_mount(XDR *xdrs, struct mount_tt *objp)
{
	return (xdr_string(xdrs, &objp->ml_name, MNTNAMLEN) &&
		xdr_string(xdrs, &objp->ml_path, MNTPATHLEN) &&
		xdr_mounts_list(xdrs, &objp->ml_nxt));
}

bool_t
xdr_mounts_list(XDR *xdrs, mounts_list *objp)
{
        return (xdr_pointer(xdrs, (char **)objp, sizeof(struct mount_tt),
                            (xdrproc_t) xdr_mount));
}

bool_t
xdr_groups(XDR *xdrs, struct groups *objp)
{
	return (xdr_string(xdrs, &objp->g_name, MNTNAMLEN) &&
		xdr_groups_list(xdrs, &objp->g_next));
}

bool_t
xdr_groups_list(XDR *xdrs, groups_list *objp)
{
	return (xdr_pointer(xdrs, (char **)objp, sizeof(struct groups),
			    (xdrproc_t) xdr_groups));
}

bool_t
xdr_dev_t(XDR *xdrs, dev_t objp)
{
	return xdr_short(xdrs, (short *) &objp);
}

bool_t
xdr_exports(XDR *xdrs, struct exports *objp)
{

	return (xdr_string(xdrs, &objp->ex_name, MNTNAMLEN) &&
		xdr_groups_list(xdrs, &objp->ex_groups) &&
		xdr_exports_list(xdrs, &objp->ex_next));
}

bool_t
xdr_exports_list(XDR *xdrs, exports_list *objp)
{
	return (xdr_pointer(xdrs, (char **)objp, sizeof(struct exports),
			    (xdrproc_t) xdr_exports));
}

