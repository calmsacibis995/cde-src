
/*
 *	$XConsortium: attch_ed.h /main/cde1_maint/2 1995/10/16 11:01:41 rcs $
 *
 *	@(#)attch_ed.h	1.4 08 Oct 1994	
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
 * attch_ed.h
 *
 * Functions for manipulating the Attachments Editor
 */

#ifndef _attch_ed_h
#define _attch_ed_h

typedef enum
{
    ATTCH_ED_PARENT = 0,
    ATTCH_ED_OPPOSITE_PARENT,
    ATTCH_ED_SIBLING,
    ATTCH_ED_OPPOSITE_SIBLING,
    ATTCH_ED_GRIDLINE,
    ATTCH_ED_CENTER_GRIDLINE,
    ATTCH_ED_NONE
} ATTCH_ED_ATTACH_TYPE;

typedef enum
{
    ATTCH_ED_WRN_LOAD = 0,
    ATTCH_ED_WRN_CHANGE_OBJTYPE,
    ATTCH_ED_WRN_CLOSE
} ATTCH_ED_WRN_TYPE;

#define attch_ed_need_obj_menu(attach_type)	\
	((attach_type == ATTCH_ED_SIBLING) || (attach_type == ATTCH_ED_OPPOSITE_SIBLING))

/*
 * Attachment Editor Settings
 */
typedef struct  ATTACHMENT_EDITOR_SETTINGS
{
    Widget			prop_sheet;
    PropOptionsSettingRec	top_attach_type;
    PropFieldSettingRec		top_attach_offset;
    PropFieldSettingRec		top_attach_position;
    PropOptionsSettingRec	top_attach_obj;
    PropOptionsSettingRec	bottom_attach_type;
    PropFieldSettingRec		bottom_attach_offset;
    PropFieldSettingRec		bottom_attach_position;
    PropOptionsSettingRec	bottom_attach_obj;
    PropOptionsSettingRec	left_attach_type;
    PropFieldSettingRec		left_attach_offset;
    PropFieldSettingRec		left_attach_position;
    PropOptionsSettingRec	left_attach_obj;
    PropOptionsSettingRec	right_attach_type;
    PropFieldSettingRec		right_attach_offset;
    PropFieldSettingRec		right_attach_position;
    PropOptionsSettingRec	right_attach_obj;
    ABObj			cur_object;
} AttchEditorSettingsRec, *AttchEditorSettings;

void		attch_ed_init(
		);

void		attch_ed_show_dialog(
		    ABObj	cur_obj
		);

BOOL		attch_ed_can_edit_attachments(
		    ABObj	obj
		);

#endif /* _attch_ed_h */
