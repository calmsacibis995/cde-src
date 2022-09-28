/* $XConsortium: cmcbxdr.c /main/cde1_maint/1 1995/07/17 19:49:52 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)cmcbxdr.c	1.6 96/11/12 Sun Microsystems, Inc."

/*
 * xdr routines for v2 callback protocol data structures
 */

#include "cmcb.h"
#include "csa.h"
#include "cmxdr.h"


/*
 * cal_attr_data - contain names of calendar attributes updated
 */

bool_t
xdr_cmcb_cal_attr_data(register XDR *xdrs, cmcb_cal_attr_data *objp)
{

	if (!xdr_array(xdrs, (char **)&objp->names, (u_int *) &objp->num_names,
	    ~0, sizeof (cms_name), (xdrproc_t) xdr_cms_name))
		return (FALSE);
	return (TRUE);
}

/*
 * add_entry_data - contain information of the deleted entry
 */

bool_t
xdr_cmcb_add_entry_data(register XDR *xdrs, cmcb_add_entry_data *objp)
{

	if (!xdr_string(xdrs, &objp->id, ~0))
		return (FALSE);
	return (TRUE);
}

/*
 * delete_entry_data - contain information of the deleted entry
 */

bool_t
xdr_cmcb_delete_entry_data(register XDR *xdrs, cmcb_delete_entry_data *objp)
{

	if (!xdr_string(xdrs, &objp->id, ~0))
		return (FALSE);
	if (!xdr_CSA_sint32(xdrs, &objp->scope))
		return (FALSE);
	if (!xdr_long(xdrs, &objp->time))
		return (FALSE);
	return (TRUE);
}

/*
 * update_entry_data - contain information of the updated entry
 * if new entry id is not resulted from the update, oldid will be
 * set to a NULL string ("").
 */

bool_t
xdr_cmcb_update_entry_data(register XDR *xdrs, cmcb_update_entry_data *objp)
{

	if (!xdr_string(xdrs, &objp->newid, ~0))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->oldid, ~0))
		return (FALSE);
	if (!xdr_CSA_sint32(xdrs, &objp->scope))
		return (FALSE);
	if (!xdr_long(xdrs, &objp->time))
		return (FALSE);
	return (TRUE);
}

bool_t
xdr_cmcb_update_data(register XDR *xdrs, cmcb_update_data *objp)
{

	if (!xdr_CSA_uint32(xdrs, &objp->reason))
		return (FALSE);

	switch (objp->reason) {
	case CSA_CB_CALENDAR_ATTRIBUTE_UPDATED:
		if (!xdr_pointer(xdrs, (char **)&objp->data.cdata,
		    sizeof (cmcb_cal_attr_data),
		    (xdrproc_t) xdr_cmcb_cal_attr_data))
			return (FALSE);
		break;
	case CSA_CB_ENTRY_ADDED:
		if (!xdr_pointer(xdrs, (char **)&objp->data.adata,
		    sizeof (cmcb_add_entry_data),
		    (xdrproc_t) xdr_cmcb_add_entry_data))
			return (FALSE);
		break;
	case CSA_CB_ENTRY_DELETED:
		if (!xdr_pointer(xdrs, (char **)&objp->data.ddata,
		    sizeof (cmcb_delete_entry_data),
		    (xdrproc_t) xdr_cmcb_delete_entry_data))
			return (FALSE);
		break;
	case CSA_CB_ENTRY_UPDATED:
		if (!xdr_pointer(xdrs, (char **)&objp->data.udata,
		    sizeof (cmcb_update_entry_data),
		    (xdrproc_t) xdr_cmcb_update_entry_data))
			return (FALSE);
		break;
	}
	return (TRUE);
}

bool_t
xdr_cmcb_update_callback_args(register XDR *xdrs, cmcb_update_callback_args *objp)
{

	if (!xdr_string(xdrs, &objp->calendar, ~0))
		return (FALSE);
	if (!xdr_string(xdrs, &objp->user, ~0))
		return (FALSE);
	if (!xdr_cmcb_update_data(xdrs, &objp->data))
		return (FALSE);
	return (TRUE);
}
