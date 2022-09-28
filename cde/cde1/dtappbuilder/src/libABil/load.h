
/*
 *	$XConsortium: load.h /main/cde1_maint/2 1995/10/16 10:16:25 rcs $
 *
 *	@(#)load.h	1.13 09 Sep 1994	cde_app_builder/src/libABil
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
 * load.h - public functions to load a file.
 */
#ifndef _ABIL_BIL_LOADFILE_H_
#define _ABIL_BIL_LOADFILE_H_

#include <ab_private/istr.h>

/*
 * Functions
 */
extern STRING   abil_loadmsg(STRING);   /* output error message */
extern int      abil_loadmsg_clear(void);
extern int      abil_loadmsg_set_object(STRING objname);
extern int      abil_loadmsg_set_att(STRING attname);
extern int      abil_loadmsg_set_action_att(STRING actattname);
extern int      abil_loadmsg_set_file(STRING bilfile);
extern int 	abil_print_load_err(int error_num);
extern int 	abil_print_custom_load_err(STRING error_msg);
extern int	abil_loadmsg_set_err_printed(BOOL); 
extern BOOL 	abil_loadmsg_err_printed(void);

#endif /* _ABIL_BIL_LOADFILE_H_ */

