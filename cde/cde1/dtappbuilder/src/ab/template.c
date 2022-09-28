
/*
 *	$XConsortium: template.c /main/cde1_maint/2 1995/10/16 14:02:20 rcs $
 *
 * @(#)template.c	1.7 01/20/96 cde_app_builder/src/ab
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

/*
 * File: template.c - C template for this directory
 */

#define	FOO_DEFINES	"here"

const char *foo_consts = "also here";

/*************************************************************************
**                                                                      **
**       Private Function Declarations                                  **
**                                                                      **
**************************************************************************/
static int	foo(
		    int	  foo_int,
		    char *foo_char,
		);


/*************************************************************************
**                                                                      **
**       Data     	                                                **
**                                                                      **
**************************************************************************/
extern int	extern_foo;

static int	private_foo;


/*************************************************************************
**                                                                      **
**       Function Definitions                                           **
**                                                                      **
**************************************************************************/

int 
ab_foo(
    char	*foo_char
)
{

}

static int
foo(
    int		foo_int,
    char	*foo_char
)
{

}
