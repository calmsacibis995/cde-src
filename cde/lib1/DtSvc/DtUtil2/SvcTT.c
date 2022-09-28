/* $XConsortium: SvcTT.c /main/cde1_maint/1 1995/07/17 18:16:41 drk $ */
/*%% 									 */
/*%%  	RESTRICTED CONFIDENTIAL INFORMATION:                             */
/*%% 									 */
/*%% 	The information in this document is subject to special		 */
/*%% 	restrictions in a confidential disclosure agreement between	 */
/*%% 	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this	 */
/*%% 	document outside HP, IBM, Sun, USL, SCO, or Univel without	 */
/*%% 	Sun's specific written approval.  This document and all copies	 */
/*%% 	and derivative works thereof must be returned or destroyed at	 */
/*%% 	Sun's request.							 */
/*%% 									 */
/*  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*  (c) Copyright 1993, 1994 International Business Machines Corp. 	 */
/*  (c) Copyright 1993, 1994 Sun Microsystems, Inc. 			 */
/*  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%% 									 */
#include <sys/time.h>
#include <Dt/SvcTT.h>


/*
 * Ensure that there is a valid ToolTalk connection open.
 * If none is open, creates one and adds it to the XtAppContext
 * of widget.  Returns
 *   TT_OK		Success.
 *   TT_ERR_NOMP	ToolTalk could not be initialized.
 *   TT_ERR_POINTER	widget was null.
 */
Tt_status
_DtSvcInitToolTalk(
	Widget widget
)
{
	char	       *procid;
	Tt_status	status;
	int		fd;
	XtAppContext	ctxt;

	procid = tt_default_procid();
	status = tt_ptr_error( procid );
	if (status == TT_OK) {
		tt_free( procid );
	}
	if ((status == TT_ERR_NOMP) || (status == TT_ERR_PROCID)) {
		if (widget == 0) {
			return TT_ERR_POINTER;
		}
		procid = tt_open(); /* do not tt_free(); Xt will pass it back */
		status = tt_ptr_error( procid );
		if (status != TT_OK) {
			return status;
		}
		fd = tt_fd();
		status = tt_int_error( fd );
		if (status != TT_OK) {
			return status;
		}
		ctxt = XtWidgetToApplicationContext( widget );
		XtAppAddInput( ctxt, fd, (XtPointer)XtInputReadMask,
			       tttk_Xt_input_handler, procid );
	}
	return TT_OK;
}
