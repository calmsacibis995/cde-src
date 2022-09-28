/* $XConsortium: catio.h /main/cde1_maint/1 1995/07/17 20:32:43 drk $ */
/*
 * COMPONENT_NAME: CMDMSG
 *
 * FUNCTIONS: catio.h
 *
 * ORIGINS: 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */
/*
 * @OSF_COPYRIGHT@
 */
/* 
 * (c) Copyright 1993, 1994 Hewlett-Packard Company  
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc. 
 * (c) Copyright 1993, 1994 Novell, Inc. 
 */

/*
 * Note: There is a duplicate (sort of) of this file in libc.
 * 
 * If you change any values here, make sure the other also gets
 * changed.
 */

#ifndef _BLD	/* bootstrap indicator */
#include "mesg.h"
#endif /* _BLD */
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>

#define ERR 		(-1)
#define TRUE 		1
#define FALSE 		0

#define QTSTR		"$quote"
#define SETSTR		"$set"

#define PATH_FORMAT	"/usr/lib/nls/%L/msg/%N:/usr/lib/nls/prime/msg/%N"
#define DEFAULT_LANG	"C"

#define SETMIN		1
#define SETMAX  	65535

#define die(s)			puts(s), exit(1)
