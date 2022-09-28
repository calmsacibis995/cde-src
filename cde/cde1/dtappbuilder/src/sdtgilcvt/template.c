
/*
 *	$Revision: 1.1 $
 *
 *	@(#)template.c	1.2 01/20/95	cde_app_builder/src/dtgilcvt
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */
#pragma ident "@(#)template.c	1.2 95/01/20 SMI"

/*
 *  template.c - template c file.
 */

#include "template.h"

/*************************************************************************
**                                                                      **
**       Constants (#define and const)					**
**                                                                      **
**************************************************************************/
#define	FOO_DEFINES	"here"

const char *foo_consts = "also here";

/*************************************************************************
**                                                                      **
**       Private Functions (C declarations and macros)			**
**                                                                      **
**************************************************************************/
#define foo_macro(x) ((x) + 1)

static int	foo_me(
		    int	  foo_int,
		    char *foo_char
		);


/*************************************************************************
**                                                                      **
**       Data     	                                                **
**                                                                      **
**************************************************************************/
/* static int	foo_private= 0; */


/*************************************************************************
**                                                                      **
**       Function Definitions                                           **
**                                                                      **
**************************************************************************/

/* int 
foo_func(
    char	*foo_char
)
{

} */

/* static int
foo_me(
    int		foo_int,
    char	*foo_char
)
{

} */

