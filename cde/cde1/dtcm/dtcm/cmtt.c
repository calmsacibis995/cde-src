/*******************************************************************************
**
**  cmtt.c
**
**  $XConsortium: cmtt.c /main/cde1_maint/2 1995/09/06 08:03:15 lehors $
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

#pragma ident "@(#)cmtt.c	1.17 96/11/12 Sun Microsystems, Inc."

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Tt/tttk.h>
#include "calendar.h"
#include "props.h"
#include "misc.h"
#include "cmfns.h"
#include "debug.h"


static Tt_message load_cb();

#ifdef FNS_DEMO
static const char *ptype = "Sun_DtCm";

static void
cm_iconic(
	Widget	win,
	int	iconic)

{
	if (iconic) {
		XIconifyWindow(XtDisplay(win),
				XtWindow(win),
				XScreenNumberOfScreen(XtScreen(win)));
	} else {
		XtPopup(win, XtGrabNone);
		XMapWindow(XtDisplay(win), XtWindow(win));
	}

	return;
}
#endif

/* 
 * Initialize tooltalk.  Can be called multiple times: the first call
 * initializes tooltalk, subsequent calls are no-ops.
 *
 * Returns
 *		-1	Error.  Tooltalk not initialized
 *		0	Tooltalk already initialized
 *		1	Tooltalk succussfully intialized
 */
int
cmtt_init(
	char		*toolname,
	Calendar	*c,
	XtAppContext	context,
	Widget		 shell)

{
	int		ttfd;
	Tt_status	status;
	char		*ttenv;
	char		*session;

	if (c->tt_procid) {
		return 0;
	}

	ttenv = (char *)getenv("TT_SESSION");
	if (!ttenv || strlen(ttenv) == 0) {
		session = tt_X_session(XDisplayString(c->xcontext->display));
		tt_default_session_set(session);
		tt_free(session);
	}
	c->tt_procid = ttdt_open(&ttfd, toolname, "SunSoft", "%I", 1);

	if (tt_is_err(tt_ptr_error(c->tt_procid))) {
		c->tt_procid = NULL;
		return -1;
	}

#ifdef FNS_DEMO
	/*
	 * Declare our ptype, and register the callback to handle
	 * Edit/Display/Compose requests
	 */
	status = ttmedia_ptype_declare(ptype, 0, load_cb, (void *)c, 1);

	if (tt_is_err(status)) {
		fprintf(stderr, "cmtt_init could not declare ptype: %s\n",
			tt_status_message(status));
	}
#endif

	ttdt_session_join(0, NULL, shell, c, 1);

	XtAppAddInput(context, ttfd, (XtPointer)XtInputReadMask,
			tttk_Xt_input_handler, c->tt_procid);

	/* Process the message that started us, if any */
	tttk_Xt_input_handler( 0, 0, 0 );

	return 1;
}

#ifdef FNS_DEMO
static Tt_message
contract_cb(
	Tt_message	msg,
	Tttk_op		op,
	Widget		shell,
	void		*clientdata,
	Tt_message	contract)
{

	DP(("contract_cb(msg=%d, op=%d, shell=%d, data=%d, cntrct=%d)\n",
		msg, op, shell, clientdata, contract));

	switch (op) {
	case TTDT_QUIT:
	case TTDT_GET_STATUS:
	case TTDT_PAUSE:
	case TTDT_RESUME:
		break;
	}
	/* For now do nothing */
	return msg;
}

/*
 * Handle Edit, Display and Compose requests
 */
static Tt_message
load_cb(
	Tt_message	msg,
	void		*clientdata,
	Tttk_op		op,
	Tt_status	diagnosis,
	unsigned char	*contents,
	int		len,
	char 		*file,
	char 		*docname
)

{
	Tt_status status;
	char	*p;
	char	addr_buf[MAXNAMELEN];
	Calendar	*c;
	Tt_pattern	*contract_pats;

	DP(("load_cb(msg=%d, op=%d, diag=%d, contents=%d, len=%d, file=%s, \
docname=%s, data=%d)\n",
		msg, op, diagnosis, contents, len, file ? file : "<nil>",
		docname ? docname : "<nil>", clientdata));

	c = (Calendar *)clientdata;

	if (diagnosis != TT_OK) {
		if (tt_message_status(msg) == TT_WRN_START_MESSAGE) {
			/*
			 * Error in start message!  we may want to exit
			 * here, but for now let toolkit handle error
			 */
			 return msg;
		}

		/* Let toolkit handle the error */
		return msg;
	}

	contract_pats = ttdt_message_accept(msg, contract_cb, clientdata,
						c->frame, 1, 1);

	if ((status = tt_ptr_error(contract_pats)) != TT_OK) {
		fprintf(stderr, "dtcm: load_cb could not accept message: %s\n",
			tt_status_message(status));
	} else {
	/*
		tttk_patterns_destroy(contract_pats);
	*/
	}

	if (file != NULL) {
		p = file;
	} else if (len > 0) {
		p = (char *)contents;
	} else {
		tttk_message_fail( msg, TT_DESKTOP_ENODATA, 0, 1 );
		p = NULL;
	}

	if (p) {
		if (cmfns_use_fns((Props *)c->properties) &&
		    cmfns_lookup_calendar(p, addr_buf, sizeof(addr_buf)) > 0) {
			cm_iconic(c->frame, 0);
#ifndef NOTHREADS
			switch_it2(c, p, addr_buf, main_win);
#else
			switch_it(c, p, addr_buf, main_win);
#endif
		} else {
			/* if not registered, try calendar name itself */
			cm_iconic(c->frame, 0);
#ifndef NOTHREADS
			switch_it2(c, p, p, main_win);
#else
			switch_it(c, p, p, main_win);
#endif
		}


	}

	tt_message_reply(msg);

	tt_free((caddr_t)contents);
	tt_free(file);
	tt_free(docname);

	return 0;
}
#endif
