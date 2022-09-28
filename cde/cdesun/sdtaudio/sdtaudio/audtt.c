/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)audtt.c	1.2 96/11/20 SMI"


#include <errno.h>
#include <nl_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <Tt/tttk.h>
#include <Xm/Xm.h>

#include "sdtaudio.h"
#include "utils.h"
#include "extern.h"

/* For tooltalk initialization. */
#define	AUDTT_NAME	"CDE"
#define	AUDTT_VER	"1.1"
#define AUDTT_PTYPE	"Sun_CDEAudio"

extern int errno;

Tt_message
audtt_callback(Tt_message	msg,
	       void	       *clientdata,
	       Tttk_op		op,
	       Tt_status	diagnosis,
	       unsigned char   *contents,
	       int		len,
	       char	       *file,
	       char	       *docname)
{
	Tt_status	status;
	WindowData	*wd = (WindowData *) clientdata;

	switch(op) {

	case TTME_DISPLAY:

		if (file != NULL) {
			LoadFile(wd, file);
		}

		break;

	default:
		status = tttk_message_fail(msg, TT_DESKTOP_ENODATA, 0, 1);
		if (status != TT_OK)
			fprintf(stderr, catgets(msgCatalog, 1, 20, "%s: could not send tooltalk fail message - %s\n"), programName, tt_status_message(status));
	}

	return 0;
}


/* Function	:	audtt_decl_ptype()
 * Objective	:	Declare the process types for the CDE audio tool.
 * Arguments	:	WindowData *	- the sdtaudio structure.
 * Return Value	:	None.
 */

static void
audtt_decl_ptype(WindowData *wd)
{
	Tt_status	status;

	status = ttmedia_ptype_declare(AUDTT_PTYPE, 0, audtt_callback, 
				       wd, 0);

	if (tt_is_err(status)) {
		fprintf(stderr, catgets(msgCatalog, 1, 8,
			 "%s: could not declare tooltalk ptypes - %s\n"), 
			 programName, tt_status_message(status));
	}
}

/* Function	:	audtt_init()
 * Objective	:	Initialize tooltalk and media types supported,
 *			so that sdtaudi can be started using tooltalk.
 * Arguments	:	char *		- the toolname for cde audio
 * Return value :	Boolean		- True if tooltalk was successfully
 *					  initialized, false otherwise.
 */

Boolean
audtt_init(char *toolname, WindowData *wd)
{
	char           *procid;
	int		ttfd;

	Tt_status       status;
	Tt_pattern     *pattern;
	char           *session, *ttenv = NULL;

	/* Open and initialize a tooltalk descriptor. */
	procid = ttdt_open(&ttfd, toolname, AUDTT_NAME,
			   AUDTT_VER, True);
	if (tt_is_err(tt_ptr_error(procid)))
		return(False);

	/* Declare the process types */
	audtt_decl_ptype(wd);

	/* Join the tooltalk session. */
	pattern = ttdt_session_join(NULL, NULL, NULL, NULL, True);
	status = tt_ptr_error(pattern);
	if (tt_is_err(tt_ptr_error(pattern))) {
		fprintf(stderr, catgets(msgCatalog, 1, 9,
			 "%s: could not join tooltalk sessio - %s\n"), 
			 programName, tt_status_message(status));
		return False;
	}

	/*
	 * Catch any input on tt file descriptor.
	 */
	XtAppAddInput(appContext, ttfd, (XtPointer) XtInputReadMask,
		      tttk_Xt_input_handler, NULL);

	/*
	 * Process the message that started us, if any. This needs to
	 * go AFTER ptype declaration and XtAppAddInput()!
	 */
	tttk_Xt_input_handler(0, 0, 0);

	return True;
}
