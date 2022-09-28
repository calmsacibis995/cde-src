/*******************************************************************************
**
**  resource.h
**
**  #pragma ident "@(#)resource.h	1.8 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: resource.h /main/cde1_maint/1 1995/07/14 23:19:58 drk $
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

#ifndef _RESOURCE_H
#define _RESOURCE_H
 
#include "ansi_c.h"
#include <sys/types.h>

/*
**  Resource structure
*/
struct Resource {
	char		*resource_name;
	char		*resource_value;
	struct Resource	*next;
};
typedef struct Resource Resource;

/*
**  Function declarations
*/
extern void		free_resources	(Resource*);
extern char	    	*get_resource	(Resource*, char *, char*,
							char*, char*);
extern char		*get_resource_by_val(Resource*, char*, char*);
extern boolean_t	load_resources	(Resource**, char*);
extern boolean_t	save_resources	(Resource*, char*);
extern boolean_t	set_resource	(Resource**, char*, char*,
							char*, char*);
extern boolean_t	set_resource_by_val(Resource**, char*, char*);

#endif
