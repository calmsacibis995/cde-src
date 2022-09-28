/* $XConsortium: FormatMan.c /main/cde1_maint/2 1995/10/08 17:17:56 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:	Format.c
 **
 **   Project:     Text Graphic Display Library
 **
 **   Description: This body of formats the input into a form understood by
 **		a Display Area.
 **
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * system includes
 */
#include <limits.h>
#include <nl_types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


/*
 * private includes
 */
#include "Canvas.h"
#include "FormatManCCDFI.h"
#include "FormatManI.h"

#ifdef NLS16
#endif

/******************************************************************************
 *
 * Private variables and defines.
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Private Functions
 *
 *****************************************************************************/
/******************************************************************************
 *
 * Public Functions
 *
 *****************************************************************************/
/*********************************************************************
 * Function: _DtHelpFormatManPage
 *
 *    _DtHelpFormatManPage formats a man page
 *	into a form understood by a display area.
 *
 *********************************************************************/
int
#ifdef _NO_PROTO
_DtHelpFormatManPage(canvas_handle, man_spec, ret_topic)
	CanvasHandle	canvas_handle;
	char		*man_spec;
	TopicHandle	*ret_topic;
#else
_DtHelpFormatManPage(
	CanvasHandle	canvas_handle,
	char		*man_spec,
	TopicHandle	*ret_topic)
#endif /* _NO_PROTO */
{
    return (_CEFormatManCCDF(canvas_handle, man_spec, ret_topic));

}  /* End _DtHelpFormatManPage */
