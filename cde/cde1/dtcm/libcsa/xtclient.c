/* $XConsortium: xtclient.c /main/cde1_maint/1 1995/07/17 20:05:01 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)xtclient.c	1.9 97/01/31 Sun Microsystems, Inc."

#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <rpc/rpc.h>
#if defined(SunOS)
#include <dlfcn.h>
#include <sys/utsname.h>
#endif /* SunOS */
#include "agent_p.h"
#include "debug.h"
#include <pthread.h>

#if defined(SunOS)

/* use dlopen to access libXt instead of being dependent on it */
#define XT_LIB_NAME		"libXt.so"
#define X11_LIB_NAME		"libX11.so"
#define ADD_INPUT_FUNC_NAME	"XtAppAddInput"

typedef XtInputId (*_AddInputfptr)(XtAppContext, int, XtPointer,
			XtInputCallbackProc, XtPointer);
#endif /* SunOS */

typedef struct _appctlist {
	XtAppContext	appct;
	struct _appctlist *next;
} AppCtList;

/* registered_appct_lock protects registered_appct */
static pthread_mutex_t	registered_appct_lock;
/* xt_lock wraps call to XtAppAddInput */
static pthread_mutex_t		xt_lock;

static AppCtList *registered_appct = NULL;

/*****************************************************************************
 * forward declaration of static functions
 *****************************************************************************/
static void xtcallback(XtPointer data, int *fid, XtInputId *id);
static boolean_t new_appct(XtAppContext apptct);
static void _addinput(XtAppContext appct, int fd);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

/*
 * register callback for all file descriptors that's set
 * (since we don't know which one is ours).
 */
extern void
_DtCm_register_xtcallback(XtAppContext appct)
{
	int		i, j;
	fd_mask		fmask, *bits;
	fd_set		fdset;

	DP(("xtclient.c: _DtCm_register_xtcallback()\n"));

	if (new_appct(appct) == _B_FALSE)
		return;

	fdset = svc_fdset;
	bits = fdset.fds_bits;
	for (i = 0; i < FD_SETSIZE; i += NFDBITS) {
		fmask = *bits;
		for (j = 0; fmask != 0; j++, fmask >>= 1) {
			if (fmask & 0x1) {

				if ((i + j) >= FD_SETSIZE)
					break;

				_addinput(appct, (i * NFDBITS) + j);
			}
		}
		bits++;
	}
}

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

/*
 * callback for rpc events
 */
static void
xtcallback(XtPointer data, int *fid, XtInputId *id)
{
	fd_set rpc_bits;

	DP(("xtcallback called\n"));

	FD_ZERO(&rpc_bits);
	FD_SET(*fid, &rpc_bits);

	_DtCm_lock_get_rpc_request();
	svc_getreqset(&rpc_bits);
	_DtCm_unlock_get_rpc_request();
}

static boolean_t
new_appct(XtAppContext appct)
{
	AppCtList	*lptr;
	boolean_t	newappct = _B_TRUE;

	/* since we don't expect this to be a long list,
	 * locking the whole thing is ok
	 */
	pthread_mutex_lock(&registered_appct_lock);

	for (lptr = registered_appct; lptr != NULL; lptr = lptr->next) {
		if (lptr->appct == appct) {
			newappct = _B_FALSE;
			break;
		}
	}

	if (newappct == _B_TRUE) {
		lptr = (AppCtList *)calloc(1, sizeof(AppCtList));
		lptr->appct = appct;
		lptr->next = registered_appct;
		registered_appct = lptr;
	}

	pthread_mutex_unlock(&registered_appct_lock);

	return (newappct);
}

static void
_addinput(XtAppContext appct, int fd)
{
        /* register callback with XtAppAddInput for rpc input
	 *
	 * NOTE: X11R5 is not mt-safe so we need to wrap the call
	 *	 by a mutex lock This can be removed when a mt-safe
	 *	 version of Xt is used
	 */

#if defined(SunOS)

	static void		*handle = NULL; /* handle to shared object */
	static _AddInputfptr	fptr = NULL;

	struct utsname		os;
	boolean_t		ver4 = _B_FALSE;

	if (uname(&os) != -1) {
		if (!strcmp("5.4", os.release))
			ver4 = _B_TRUE;
	}
#endif /* SunOS */

	pthread_mutex_lock(&xt_lock);

#if defined(SunOS)

	/* use dlopen to access libXt instead of being dependent on it */
	if (handle == NULL) {
		if (ver4 == _B_TRUE) {
			/* load libXt's dependencies first */
			if ((handle = dlopen(X11_LIB_NAME,
			    RTLD_GLOBAL|RTLD_LAZY)) == NULL) {
				DP(("dlopen failed on %s - %s\n", X11_LIB_NAME,
					dlerror()));
				goto done;
			}
		}

		if ((handle = dlopen(XT_LIB_NAME, RTLD_LAZY)) == NULL) {
			DP(("dlopen failed on %s - %s\n", XT_LIB_NAME,
				dlerror()));
			goto done;
		}
	}

	if (fptr == NULL) {
		if ((fptr = (_AddInputfptr)dlsym(handle, ADD_INPUT_FUNC_NAME))
		    == NULL) {
			DP(("dlsym failed on %s - %s\n", ADD_INPUT_FUNC_NAME,
				dlerror()));
			goto done;
		}
	}

	(*fptr)(appct, fd, (XtPointer)XtInputReadMask, xtcallback, NULL);

#else /* SunOS */

	XtAppAddInput(appct, fd, (XtPointer)XtInputReadMask, xtcallback, NULL);

#endif /* SunOS */

done:

	pthread_mutex_unlock(&xt_lock);

	DP(("xtclient.c: id %d for input at fd %d\n", id, fd));
}

