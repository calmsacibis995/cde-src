/*******************************************************************************
**
**  dtfns.c
**
**  $XConsortium: dtfns.c /main/cde1_maint/1 1995/07/14 23:18:33 drk $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#pragma ident "@(#)dtfns.c	1.8 96/11/12 Sun Microsystems, Inc."

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved
 */

#ifdef FNS

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpcsvc/nis.h>
#include <rpcsvc/nislib.h>
#include <rpcsvc/ypclnt.h>
#include <dlfcn.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <xfn/xfn.h>

#include "debug.h"

#include "util.h"
#include "dtfns.h"

#include <synch.h>	/* for mutex_t */

extern char *strdup(const char *);

/* Select correct library */

#if defined(FNS_VERS)

#if FNS_VERS == 1
#define XFN1ENV
#else
#define XFN2ENV
#endif /* FNS_VERS */

#endif /* defined */

/*
 * Enable compilation for either XFN1 (pre-2.6) or XFN2 environment.
 */
#ifdef	XFN1ENV
#define	XFN1(x) /* cstyle */, x
#define	XFN2(x)
#define	_fn_ctx_handle_from_initial_with_uid(uid, auth, status) \
	    fn_ctx_handle_from_initial(status)
#else
#define	XFN1(x)
#define	XFN2(x) x,
#endif

static void	*libxfn_handle = NULL;
static int	initialized = 0;

/*
 * These are dynamically bound entry points into libxfn.  We dlopen()
 * libxfn so that cm can still run even if libxfn does not exist
 */

/* FN_ctx_t functions */
static FN_ctx_t	   *(*fn_ctx_handle_from_initial_p)(XFN2(unsigned int)
						    FN_status_t *);

static FN_ref_t	   *(*fn_ctx_lookup_p)(FN_ctx_t *,
				       const FN_composite_name_t *,
				       FN_status_t *);
static int	   (*fn_ctx_bind_p)(FN_ctx_t *,
				    const FN_composite_name_t *,
				    const FN_ref_t *,
				    unsigned int,
				    FN_status_t *);

/* FN_status_t functions */
static FN_status_t *(*fn_status_create_p)(void);
static void	    (*fn_status_destroy_p)(FN_status_t *);
static unsigned int (*fn_status_code_p)(FN_status_t *);

/* FN_ref_t functions */
static FN_ref_t		*(*fn_ref_create_p)(const FN_identifier_t *);
static void		(*fn_ref_destroy_p)(FN_ref_t *);
static const FN_identifier_t	*(*fn_ref_type_p)(const FN_ref_t *);
static const FN_ref_addr_t 	*(*fn_ref_first_p)(const FN_ref_t *, void **);
static const FN_ref_addr_t 	*(*fn_ref_next_p)(const FN_ref_t *, void **);
static int		(*fn_ref_delete_addr_p)(FN_ref_t *, void **);
static int		(*fn_ref_append_addr_p)(FN_ref_t *, const FN_ref_addr_t *);

/* FN_ref_addr_t functions */
static void		(*fn_ref_addr_destroy_p)(FN_ref_addr_t *);
static FN_ref_addr_t 	*(*fn_ref_addr_create_p)(const FN_identifier_t *,
					       size_t,
					       const void *);
static const void *	(*fn_ref_addr_data_p)(const FN_ref_addr_t *);
static size_t	(*fn_ref_addr_length_p)(const FN_ref_addr_t *);
static const FN_identifier_t *(*fn_ref_addr_type_p)(const FN_ref_addr_t*);

/* FN_composite_name_t */
static FN_composite_name_t * (*fn_composite_name_from_str_p)
	(const unsigned char *);

#ifdef XFN1ENV
static FN_composite_name_t * (*fn_composite_name_from_string_p)(
		const FN_string_t *);
/* FN_string_t */
static FN_string_t * (*fn_string_from_str_p)(const unsigned char *str);
static void (*fn_string_destroy_p)(FN_string_t *);
#endif /* XFN1ENV */

static void		(*fn_composite_name_destroy_p)(FN_composite_name_t *);

#define dtfns_check_symbol(x)	if ((x) == NULL) goto symbol_not_found;

#ifdef XFN1ENV
static FN_composite_name_t *
__fn_composite_name_from_str(const unsigned char *s)
{
	FN_string_t *fstr = (*fn_string_from_str_p)(s);
	FN_composite_name_t *cname = (*fn_composite_name_from_string_p)(fstr);
	(*fn_string_destroy_p)(fstr);
	return (cname);
}
#endif /* XFN1ENV */

/*
 * Get the initial context.  This routine caches the initial context.
 * The first call to this routine may be costly, be subsequent calls
 * are very cheap.
 */
static FN_ctx_t *
dtfns_get_initial_ctx(unsigned int authoritative)
{
	static FN_ctx_t	*initial_ctx[2] = {NULL, NULL};
	static mutex_t dtfns_get_initial_lock = DEFAULTMUTEX;

	DP(("dtfns_get_initial_ctx: Getting initial context\n"));

	if (initial_ctx[authoritative] == NULL) {
		FN_status_t *status;
		mutex_lock(&dtfns_get_initial_lock);

		status = (*fn_status_create_p)();
		/* get non-authoritative initial context handle */
		initial_ctx[authoritative] = (*fn_ctx_handle_from_initial_p)
			(XFN2(authoritative) status);
		(*fn_status_destroy_p)(status);
		mutex_unlock(&dtfns_get_initial_lock);
	}
	return (initial_ctx[authoritative]);
}

/*
 * Initialization.
 *
 * Load libxfn (with dlopen) and bind in the entry points we use (with dlsym).
 *
 * We dynamically load the FNS library so that we don't have an explicit
 * dependency  on it.  This lets us run on systems which do not have FNS
 * installed
 *
 * Returns
 *		0	FNS not available
 *		1	Success
 */

int
dtfns_init(void)
{
	static mutex_t dtfns_libxfn_lock = DEFAULTMUTEX;
	char	*libxfn = FNS_LIBRARY;
	int	error;	

	if (libxfn_handle != NULL) {
		return 1;
	}
	if (initialized)
		return (libxfn_handle != NULL);

	mutex_lock(&dtfns_libxfn_lock);
	initialized = TRUE;

	libxfn_handle = dlopen(libxfn, RTLD_LAZY);
	if (libxfn_handle == NULL) {
#ifdef CM_DEBUG
		char	*s = dlerror();
		if (s == NULL) {
			DP(("Could not dlopen %s\n", libxfn));
		} else {
			DP(("Could not dlopen %s: %s\n", libxfn, s));
		}
#endif
		mutex_unlock(&dtfns_libxfn_lock);
		return 0;
	}
	/* FN_ctx_t functions */
	fn_ctx_handle_from_initial_p = (FN_ctx_t *(*)(XFN2(unsigned int)
						      FN_status_t *))
		dlsym(libxfn_handle, "fn_ctx_handle_from_initial");
	dtfns_check_symbol(fn_ctx_handle_from_initial_p);

	fn_ctx_lookup_p = (FN_ref_t *(*)(FN_ctx_t *,
					 const FN_composite_name_t *,
					 FN_status_t *))
		dlsym(libxfn_handle, "fn_ctx_lookup");
	dtfns_check_symbol(fn_ctx_lookup_p);

	fn_ctx_bind_p = (int(*)(FN_ctx_t *,
				const FN_composite_name_t *,
				const FN_ref_t *,
				unsigned int,
				FN_status_t *))
		dlsym(libxfn_handle, "fn_ctx_bind");
	dtfns_check_symbol(fn_ctx_bind_p);

	/* FN_status_t functions */
	fn_status_create_p = (FN_status_t *(*)(void))
		dlsym(libxfn_handle, "fn_status_create");
	dtfns_check_symbol(fn_status_create_p);

	fn_status_destroy_p = (void(*)(FN_status_t *))
		dlsym(libxfn_handle, "fn_status_destroy");
	dtfns_check_symbol(fn_status_destroy_p);

	fn_status_code_p = (unsigned int(*)(FN_status_t *))
		dlsym(libxfn_handle, "fn_status_code");
	dtfns_check_symbol(fn_status_code_p);

	/* FN_ref_t functions */
	fn_ref_create_p = (FN_ref_t *(*)(const FN_identifier_t *))
		dlsym(libxfn_handle, "fn_ref_create");
	dtfns_check_symbol(fn_ref_create_p);

	fn_ref_destroy_p = (void(*)(FN_ref_t *))
		dlsym(libxfn_handle, "fn_ref_destroy");;
	dtfns_check_symbol(fn_ref_destroy_p);

	fn_ref_type_p = (const FN_identifier_t *(*)(const FN_ref_t *))
			dlsym(libxfn_handle, "fn_ref_type");
	dtfns_check_symbol(fn_ref_type_p);

	fn_ref_first_p = (const FN_ref_addr_t *(*)(const FN_ref_t *, void **))
		dlsym(libxfn_handle, "fn_ref_first");
	dtfns_check_symbol(fn_ref_first_p);

	fn_ref_next_p = (const FN_ref_addr_t *(*)(const FN_ref_t *, void **))
		dlsym(libxfn_handle, "fn_ref_next");
	dtfns_check_symbol(fn_ref_next_p);

	fn_ref_delete_addr_p = (int(*)(FN_ref_t *, void **))
		dlsym(libxfn_handle, "fn_ref_delete_addr");
	dtfns_check_symbol(fn_ref_delete_addr_p);

	fn_ref_append_addr_p = (int(*)(FN_ref_t *, const FN_ref_addr_t *))
		dlsym(libxfn_handle, "fn_ref_append_addr");
	dtfns_check_symbol(fn_ref_append_addr_p);

	/* FN_ref_addr_t functions */
	fn_ref_addr_destroy_p = (void(*)(FN_ref_addr_t *))
		dlsym(libxfn_handle, "fn_ref_addr_destroy");
	dtfns_check_symbol(fn_ref_addr_destroy_p);

	fn_ref_addr_create_p = (FN_ref_addr_t *(*)
			     (const FN_identifier_t *, size_t, const void *))
		dlsym(libxfn_handle, "fn_ref_addr_create");
	dtfns_check_symbol(fn_ref_addr_create_p);

	fn_ref_addr_data_p = (const void *(*)(const FN_ref_addr_t *))
			dlsym(libxfn_handle, "fn_ref_addr_data");
	dtfns_check_symbol(fn_ref_addr_data_p);

	fn_ref_addr_length_p = (size_t(*)(const FN_ref_addr_t *))
		dlsym(libxfn_handle, "fn_ref_addr_length");
	dtfns_check_symbol(fn_ref_addr_length_p);

	fn_ref_addr_type_p = (const FN_identifier_t *(*)
			      (const FN_ref_addr_t *))
		dlsym(libxfn_handle, "fn_ref_addr_type");
	dtfns_check_symbol(fn_ref_addr_type_p);

	/* FN_composite_name_t */
#ifdef XFN1ENV
	fn_composite_name_from_str_p = &__fn_composite_name_from_str;
	fn_composite_name_from_string_p =
		(FN_composite_name_t *(*)(const FN_string_t *))
		dlsym(libxfn_handle, "fn_composite_name_from_string");
	dtfns_check_symbol(fn_composite_name_from_string_p);

	fn_string_from_str_p = (FN_string_t * (*)(const unsigned char *str))
		dlsym(libxfn_handle, "fn_string_from_str");
	dtfns_check_symbol(fn_string_from_str_p);

	fn_string_destroy_p = (void (*)(FN_string_t *))
		dlsym(libxfn_handle, "fn_string_destroy");
	dtfns_check_symbol(fn_string_destroy_p);
#else
	fn_composite_name_from_str_p =(FN_composite_name_t *(*)(const unsigned char *))
		dlsym(libxfn_handle, "fn_composite_name_from_str");
	dtfns_check_symbol(fn_composite_name_from_str_p);
#endif /* XFN1ENV */

	fn_composite_name_destroy_p =(void(*)(FN_composite_name_t*))
		dlsym(libxfn_handle, "fn_composite_name_destroy");
	dtfns_check_symbol(fn_composite_name_from_str_p);


	mutex_unlock(&dtfns_libxfn_lock);
	return 1;

 symbol_not_found:
	mutex_unlock(&dtfns_libxfn_lock);
	return 0;
}


/*
 * Check if FNS is available for use.
 *
 * You must call dfsinit() before calling this routine.
 *
 * First call to this routine may be costly as we get the initial context.
 * Subsequent calls are very cheap.
 *
 * Returns
 *		1	Yes, FNS is available.  
 *		0	No, FNS is not available.
 *		-1	You haven't called dtfns_init().
 */

static mutex_t dtfns_available_lock = DEFAULTMUTEX;
int
dtfns_available(void)

{
	static int available;
	static int called;

	if (called) {
		return available;
	}

	mutex_lock(&dtfns_available_lock);
	if (libxfn_handle != NULL) {
		/*
		 * libxfn has been dlopened.  Now see if an FNS namespace
		 * is reachable by getting the initial context.
		 */
		if (dtfns_get_initial_ctx(0) == NULL) {
			available = 0;
		} else {
			available = 1;
		}
	} else if (initialized) {
		/* dlopen must have failed. FNS is not installed */
		available = 0;
	} else {
		available = -1;
	}
	called = 1;
	mutex_unlock(&dtfns_available_lock);

	return available;
}


static int
valid_cal_ref_type(const FN_ref_t *ref, const char *target)
{
	const FN_identifier_t *reftype = (*fn_ref_type_p)(ref);
	return (reftype != NULL &&
		reftype->length == strlen(target) &&
		strncmp((char *)reftype->contents, target, reftype->length) == 0);
}

static int
valid_cal_addr_type(const FN_identifier_t *atype, const char *target)
{
	return (atype != NULL &&
		atype->length == strlen(target) &&
		strncmp((char *)atype->contents, target, atype->length) == 0);
}

static int
equal_cal_addr_contents(const FN_identifier_t *atype, const char *target)
{
	return (atype != NULL &&
		atype->length == strlen(target) &&
		strncmp((char *)atype->contents, target, atype->length) == 0);
}

/* 
   Create reference if 'ref' is not null.
   Add address constructed using reftype and addr type and contents.
 */
static FN_ref_t *
dtfns_ref_from_string_addr(const char* saddr, 
			   const FN_identifier_t *reftype,
			   const FN_identifier_t *addrtype,
			   FN_ref_t *ref)
{
	u_char	     	buffer[DTFNS_BUFSIZE];
	XDR		xdr;
	FN_ref_addr_t 	*addr;

	xdrmem_create(&xdr, (caddr_t)buffer, DTFNS_BUFSIZE, XDR_ENCODE);
	if (xdr_string(&xdr, (char**)&saddr, ~0) == FALSE) {
		xdr_destroy(&xdr);
		fprintf(stderr, "Could not XDR encode string address");
		return 0;
	}

	/* Create new reference if none was supplied */
	if (ref == NULL)
		ref = (*fn_ref_create_p)(reftype);
	if (ref) {
		addr = (*fn_ref_addr_create_p)(addrtype, xdr_getpos(&xdr), buffer);
		xdr_destroy(&xdr);
		if (addr) {
			(*fn_ref_append_addr_p)(ref, addr);
			(*fn_ref_addr_destroy_p)(addr);
			return ref;
		} else {
			(*fn_ref_destroy_p)(ref);
			return 0;
		}
	}
	xdr_destroy(&xdr);
	return (0);
}

static char *
dtfns_string_from_ref(const FN_ref_t *ref, const char *expected_addr_type)
{
	XDR	xdr;
	char 	*addr_str = NULL;
	void	* iterposn;
	const FN_ref_addr_t *addr;
	const FN_identifier_t *addr_type;

	for (addr = (*fn_ref_first_p)(ref, &iterposn);
	     addr != NULL;
	     addr = (*fn_ref_next_p)(ref, &iterposn)) {
		addr_type = (*fn_ref_addr_type_p)(addr);
		if (addr_type != NULL && 
		    valid_cal_addr_type(addr_type, expected_addr_type)) {
			xdrmem_create(&xdr, 
				      (caddr_t)(*fn_ref_addr_data_p)(addr), 
				      (*fn_ref_addr_length_p)(addr), 
				      XDR_DECODE);
			if (xdr_string(&xdr, &addr_str, ~0) == FALSE) {
				xdr_destroy(&xdr);
				return NULL;
			}
			break;
		}
	}
	return (addr_str);
}



/* Lookup FNS fullname and return its calendar address */

int
dtfns_lookup_calendar(const char *fullname, 
		      const char* cal_ref_type, 
		      const char* cal_addr_type,
		      char *cal_addr, int max_size)
{
	FN_status_t	*status;
	FN_ref_t	*ref;
	const FN_identifier_t	*atype = 0;
	FN_composite_name_t *composite_fullname;
	FN_ctx_t *init_ctx;
	int rcode = -1;
	unsigned int code;
	char *saddr;

	if (libxfn_handle == NULL) {
		return 0;
	}

	if (fullname == NULL) {
		return -1;
	}

	if ((init_ctx = dtfns_get_initial_ctx(0)) == NULL) {
		return -1;
	}

	composite_fullname = (*fn_composite_name_from_str_p)
		((const unsigned char *)fullname);
	if (composite_fullname == 0) {
		return(-1);
	}
	status = (*fn_status_create_p)();
	if (status == 0) {
		(*fn_composite_name_destroy_p)(composite_fullname);
		return(-1);
	}
	ref = (*fn_ctx_lookup_p)(init_ctx, composite_fullname, status);

	(*fn_composite_name_destroy_p)(composite_fullname);
	code = (*fn_status_code_p)(status);
	(*fn_status_destroy_p)(status);

	if (ref == 0) {
		return(-code);
	}

	if (!valid_cal_ref_type(ref, cal_ref_type)) {
		(*fn_ref_destroy_p)(ref);
		return(-1);
	}
	/* decode reference and extract calendar address */
	saddr = dtfns_string_from_ref(ref, cal_addr_type);
	(*fn_ref_destroy_p)(ref);
	if (saddr) { 
		strcpy(cal_addr, saddr);
		free(saddr);
		return (1);
	}
	return (-1);
}

/* Delete address with 'target_addr_type' from reference 'ref' */

static void
dtfns_delete_from_ref(FN_ref_t *ref, const char *target_addr_type)
{
	void *iterposn;
	const FN_ref_addr_t *addr;
	const FN_identifier_t *addr_type;
	
	for (addr = (*fn_ref_first_p)(ref, &iterposn);
	     addr != NULL;
	     addr = (*fn_ref_next_p)(ref, &iterposn)) {
		addr_type = (*fn_ref_addr_type_p)(addr);
		if (addr_type != NULL && 
		    valid_cal_addr_type(addr_type, target_addr_type)) {
			(*fn_ref_delete_addr_p)(ref, &iterposn);
			break;
		}
	}
}


/*
 * 
 *	Bind a calendar address to a name.
 *	If name is bound to another type of object (reference types are
 *	are different) leave it alone.
 * 	If calendar address does not exist, add it.
 *	If calendar address exists already and has not changed leave it alone.
 *	If calendar address exists and has been changed, replace binding.
 *
 *	name		Absolute FNS name to bind string to
 *
 *	ref_type	Reference type.  We need this to create
 *			the reference if it does not already exist.
 *	
 *	addr_type	Address types.
 *
 *	location	Calendar location
 *
 *	Returns
 *		-1	Error
 *		0	FNS not available
 *		1	Success
 */
int
dtfns_register_calendar(
	const char 	*fullname,	/* Absolute FNS name */
	const char	*ref_type,	/* Reference type */
	const char	*addr_type,	/* Address type of data to operate on */
	const char	*caladdr	/* String to bind to name */
)
{
	FN_ctx_t	*init_ctx;
	FN_status_t	*status;
	FN_ref_t        *ref = NULL;
	FN_composite_name_t *composite_fullname;
	FN_identifier_t reftype, addrtype;
	int rcode = -1;
	unsigned int code;

	if (libxfn_handle == NULL) {
		return 0;
	}

	if (fullname == NULL) {
		return -1;
	}

	if ((init_ctx = dtfns_get_initial_ctx(1)) == NULL) {
		return -1;
	}

	addrtype.format = FN_ID_STRING;
	addrtype.length = strlen(addr_type);
	addrtype.contents = (void*)addr_type;

	reftype.format = FN_ID_STRING;
	reftype.length = strlen(ref_type);
	reftype.contents = (void*)ref_type;

	composite_fullname = (*fn_composite_name_from_str_p)
		((const unsigned char *)fullname);
	if (composite_fullname == 0) {
		return(rcode);
	}

	status = (*fn_status_create_p)();
	if (status == 0) {
		(*fn_composite_name_destroy_p)(composite_fullname);
		return (rcode);
	}

	/* Lookup calendar binding */
	ref = (*fn_ctx_lookup_p)(init_ctx, composite_fullname, status);
	if (ref != NULL) {
		char *saddr = NULL;
		if (!valid_cal_ref_type(ref, ref_type)) {
			/* Already bound to something else, leave alone */
			(*fn_ref_destroy_p)(ref);
			(*fn_composite_name_destroy_p)(composite_fullname);
			(*fn_status_destroy_p)(status);
			return(1);
		}
		/* decode reference and extract calendar address */
		saddr = dtfns_string_from_ref(ref, addr_type);
		if (saddr != NULL) {
			/* Already has a calendar address, check for sameness */
			if (strcmp(saddr, caladdr) == 0) {
				/* Same, leave alone */
				(*fn_ref_destroy_p)(ref);
				(*fn_composite_name_destroy_p)(composite_fullname);
				(*fn_status_destroy_p)(status);
				free (saddr);
				return(1);
			}
			/* Different, remove from reference */
			dtfns_delete_from_ref(ref, addr_type);
		}
		/* Continue to add calendar address to binding */
	}
	ref = dtfns_ref_from_string_addr(caladdr, &reftype, &addrtype, ref);
	if (ref == 0) {
		(*fn_composite_name_destroy_p)(composite_fullname);
		(*fn_status_destroy_p)(status);
		return(0);
	}
	rcode = (*fn_ctx_bind_p)(init_ctx, composite_fullname, ref,  0, status);

	code = (*fn_status_code_p)(status);
	(*fn_status_destroy_p)(status);
	(*fn_composite_name_destroy_p)(composite_fullname);
	(*fn_ref_destroy_p)(ref);
	if (rcode <= 0)
		return (-rcode);
	return (1);
}
#endif /* FNS */
