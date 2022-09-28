/*
 *	$XConsortium: msg_fileP.h /main/cde1_maint/2 1995/10/16 11:32:18 rcs $
 *
 * @(#)msg_fileP.h	1.5 12 Oct 1994	cde_app_builder/src/abmf
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

#ifndef _ABMF_MSG_FILEP_H_
#define _ABMF_MSG_FILEP_H_

/*
 * msg_fileP.h - ADT implementation declrn for message file [re]generation.
 */

#include "msg_file.h"

#define DTB_GEN_MSG_SET_PREFIX	"DTBMS_"

/* INTERNAL: A message */
typedef struct _MsgRec {
    BOOL	is_referenced;
    int		msg_number;
    ISTRING	msg_string;	/* message string */
    ISTRING	msg_comment;	/* comment after the message: i18n guide */
} MsgRec, *Msg;


/* EXPORT: A message set */
typedef struct _MsgSetRec {
    int		set_number;
    char	*set_name;	
    BOOL	allow_msg_delete;
    BOOL	is_generated;	/* false <-> msgs are NULL; everything
						stored as set comment */
    BOOL	is_referenced;
    char	*set_comment;	/* comment after $set stmt: i18n guide */
    Msg		*msgs;
    int		num_msgs;
    int		alloced_slots;
} MsgSetRec;


/* EXPORT: Memory representation of a message file */
typedef struct _MsgFileRec {
    FILE	*fp;		/* pointer to file which was loaded */
    char	*file_comment;	/* comment before first $set stmt */
    MsgSet	*msg_sets;	/* in order read from msg_file */
    int		num_msg_sets;
    int		alloced_slots;
} MsgFileRec;

#endif /* !_ABMF_MSG_FILEP_H_ */
