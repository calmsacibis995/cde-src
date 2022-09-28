/* $XConsortium: cmxdr.h /main/cde1_maint/1 1995/07/17 19:51:06 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _CMXDR_H
#define _CMXDR_H

#pragma ident "@(#)cmxdr.h	1.6 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "csa.h"

/*
 * xdr routines for xapia csa data structures
 */

bool_t xdr_CSA_extension(XDR *xdrs, CSA_extension *objp);

bool_t xdr_CSA_date_time_entry(XDR *xdrs, CSA_date_time_entry *objp);

bool_t xdr_CSA_date_time_list(XDR *xdrs, CSA_date_time_list *objp);

bool_t xdr_CSA_calendar_user(XDR *xdrs, CSA_calendar_user *objp);

bool_t xdr_CSA_access_rights(XDR *xdrs, CSA_access_rights *objp);

bool_t xdr_CSA_access_list(XDR *xdrs, CSA_access_list *objp);

bool_t xdr_CSA_attendee(XDR *xdrs, CSA_attendee *objp);

bool_t xdr_CSA_attendee_list(XDR *xdrs, CSA_attendee_list *objp);

bool_t xdr_CSA_opaque_data(XDR *xdrs, CSA_opaque_data *objp);

bool_t xdr_CSA_reminder(XDR *xdrs, CSA_reminder *objp);

bool_t xdr_CSA_attribute_value(XDR *xdrs, CSA_attribute_value *objp);

bool_t xdr_CSA_uint32(XDR *xdrs, CSA_uint32 *objp);

bool_t xdr_CSA_sint32(XDR *xdrs, CSA_sint32 *objp);

#endif

