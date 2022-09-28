/*
 *	$XConsortium: msg_file.h /main/cde1_maint/2 1995/10/16 11:32:01 rcs $
 *
 * @(#)msg_file.h	1.1 15 Jul 1994	cde_app_builder/src/abmf
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

#ifndef _ABMF_MSG_FILE_H_
#define _ABMF_MSG_FILE_H_

/*
 * msg_file.h - ADT interface for message source file [re]generation.
 */

#include <ab/util_types.h>
#include <ab_private/obj.h>


/* A message set */
typedef struct _MsgSetRec	*MsgSet;

/* MsgSet Methods */
MsgSet	MsgSet_create(int set_number, char *name);
int	MsgSet_destroy(MsgSet that);
int	MsgSet_get_number(MsgSet that);
int	MsgSet_just_find_msg(MsgSet that, char *string);
int	MsgSet_sure_find_msg(MsgSet that, char *string);
int	MsgSet_set_msg_is_referenced(
			MsgSet	that,
			int	msg_num, 
			BOOL	msg_is_referenced
	);
int	MsgSet_set_allow_msg_delete(MsgSet that, BOOL allow_delete);
BOOL	MsgSet_get_allow_msg_delete(MsgSet that);
int	MsgSet_set_is_referenced(MsgSet that, BOOL set_is_referenced);
BOOL	MsgSet_is_referenced(MsgSet that);


/* Memory representation of a message source file */
typedef struct _MsgFileRec	*MsgFile;

/* MsgFile Methods */
MsgFile	MsgFile_create(char *project_name, char *msg_src_filename);
int	MsgFile_destroy(MsgFile that);
int	MsgFile_save(MsgFile that, FILE **msgFileOut);
MsgSet	MsgFile_just_find_msg_set(MsgFile that, char *module_name);
MsgSet	MsgFile_sure_find_msg_set(MsgFile that, char *module_name);
			/* adds one, if not already there */
MsgSet	MsgFile_obj_sure_find_msg_set(MsgFile that, ABObj obj);

#endif /* !_ABMF_MSG_FILE_H_ */
